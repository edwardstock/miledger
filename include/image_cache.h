/*!
 * miledger.
 * image_cache.h
 *
 * \date 2021
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#ifndef MILEDGER_IMAGECACHE_H
#define MILEDGER_IMAGECACHE_H

#include "errors.h"
#include "miledger-config.h"
#include "rxqt_instance.hpp"
#include "utils.h"

#include <QBuffer>
#include <QByteArray>
#include <QCache>
#include <QCoreApplication>
#include <QDir>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QString>
#include <QTemporaryDir>
#include <memory>
#include <minter/api/explorer/explorer_results.h>

class Imager {
private:
    QString m_cacheDir;
    QCache<QString, QPixmap> m_inMemory;
    std::unordered_map<QString, bool> m_loadImageLocks;
    std::mutex m_loadImageLocksMutex;
    std::mutex m_cacheLock;

#ifdef MILEDGER_APPLE
    Imager()
        : m_cacheDir(QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/../Resources/cache/images")),
          m_inMemory(1'024'000 * 20 /*20 mb enough for most cases*/) {
        QDir cd(m_cacheDir);
        if (!cd.exists(m_cacheDir)) {
            cd.mkpath(m_cacheDir);
        }
    }
#else
    Imager()
        : m_cacheDir(QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/cache/images")),
          m_inMemory(1'024'000 * 20 /*20 mb enough for most cases*/) {
        QDir cd(m_cacheDir);
        if (!cd.exists(m_cacheDir)) {
            cd.mkpath(m_cacheDir);
        }
    }
#endif

    rxcpp::observable<bool> loadImage(const QString& url, const QString& fileKey) {
        return rxcpp::observable<>::create<bool>([this, url, fileKey](rxcpp::subscriber<bool> emitter) {
            const QString imageKey = fileKey;
            const std::string imageFile = Imager::get().filePathForKey(imageKey).toStdString();

            if (exist(imageKey)) {
                qDebug() << "Return existent cached pixmap for " << imageKey;
                emitter.on_next(true);
                emitter.on_completed();
                return;
            }

            {
                // lock map to test and return error/continue job
                std::lock_guard<std::mutex> lock(m_loadImageLocksMutex);
                if (m_loadImageLocks.count(imageKey)) {
                    try {
                        throw image_already_in_work_error();
                    } catch (...) {
                        emitter.on_error(std::current_exception());
                    }
                    return;
                }

                m_loadImageLocks[imageKey] = true;
            }

            cpr::Response resp;
            cpr::Session session;
            //            qDebug() << "Loading image " << url;
            session.SetVerifySsl(cpr::VerifySsl(false));
            session.SetUrl(cpr::Url(url.toStdString()));
            curl_easy_setopt(session.GetCurlHolder()->handle, CURLOPT_NOSIGNAL, 1);

            std::ofstream tmpFile(imageFile, std::ios::binary);

            resp = session.Download(tmpFile);
            tmpFile.close();

            if (resp.status_code >= 400) {
                qDebug() << "Unable to load image " << url << ": [" << resp.status_code << "] " << resp.error.message;
                emitter.on_error(std::make_exception_ptr(
                    std::runtime_error(QString("Unable to load image %1: [%2] %3")
                                           .arg(
                                               url,
                                               QString::number(resp.status_code),
                                               QString::fromStdString(resp.error.message))
                                           .toStdString())));
                return;
            }

            const QString targetPath = QString::fromStdString(imageFile);
            qDebug() << "Load " << targetPath << " as pixmap or image";

            QPixmap pm;
            if (!pm.load(targetPath)) {
                QImage img(targetPath);
                qDebug() << "QImage size: " << img.size();
                qDebug() << "QImage is null: " << img.isNull();
                pm = QPixmap::fromImage(img);
                if (pm.isNull() || pm.width() == 0 || pm.height() == 0) {
                    {
                        std::lock_guard<std::mutex> lock(m_loadImageLocksMutex);
                        m_loadImageLocks.erase(imageKey);
                    }
                    qDebug() << "QPixmap icon is null: " << pm.isNull();
                    qDebug() << "Unable to handle image " << imageKey << ": unknown error while loading pixmap";
                    emitter.on_error(std::make_exception_ptr(
                        std::runtime_error(QString("Unable to handle image %1: unknown error while loading pixmap").arg(url).toStdString())));
                    return;
                }
            }

            if (!Imager::get().preload(imageKey)) {
                qDebug() << "Unable to preload " << imageKey << " to image cache";
                emitter.on_error(std::make_exception_ptr(
                    std::runtime_error("Unable to save image to cache")));
                return;
            }

            emitter.on_next(true);
            emitter.on_completed();

            {
                std::lock_guard<std::mutex> lock(m_loadImageLocksMutex);
                m_loadImageLocks.erase(imageKey);
            }
        });
    }

public:
    class ImageTransformer {
    private:
        int m_width = 0;
        int m_height = 0;
        Qt::AspectRatioMode m_ratio;
        Qt::TransformationMode m_mode;
        bool m_circular = false;
        bool m_onlyScaleDown = true;
        QString m_key;

    public:
        explicit ImageTransformer(QString key)
            : m_key(std::move(key)) {
        }

        ImageTransformer& scaleDownOnly() {
            m_onlyScaleDown = true;
            return *this;
        }

        ImageTransformer& resize(int w, int h, Qt::AspectRatioMode ratio = Qt::KeepAspectRatio, Qt::TransformationMode mode = Qt::SmoothTransformation) {
            m_width = w;
            m_height = h;
            m_ratio = ratio;
            m_mode = mode;
            return *this;
        }

        ImageTransformer& circular() {
            m_circular = true;
            return *this;
        }

        const QString& sourceKey() const {
            return m_key;
        }

        QString createKey() const {
            QString key = m_key;
            if (m_width >= 0 && m_height > 0) {
                key.append('_');
                key.append(QString::number(m_width));
                key.append('x');
                key.append(QString::number(m_height));
            }
            if (m_circular) {
                key.append("_circular");
            }
            return key;
        }

        rxcpp::observable<bool> transform() {
            // just copy context to avoid deletion in new thread. as we have just a couple of variables, don't worry about memory
            ImageTransformer that = *this;
            return rxcpp::observable<>::create<bool>([that](rxcpp::subscriber<bool> emitter) {
                const QString targetKey = that.createKey();
                if (Imager::get().exist(targetKey)) {
                    emitter.on_next(true);
                    emitter.on_completed();
                    return;
                }

                const QString targetPath = Imager::get().filePathForKey(targetKey);
                const QPixmap origin = *Imager::get().load(that.sourceKey());

                QSize targetSize(that.m_width > 0 ? that.m_width : origin.width(), that.m_height > 0 ? that.m_height : origin.height());
                QRect targetRect(0, 0, targetSize.width(), targetSize.height());

                bool scaleUpDetected = (that.m_onlyScaleDown && (origin.size().width() < targetSize.width() || origin.size().height() < targetSize.height()));
                if (origin.size() == targetSize || scaleUpDetected) {
                    // workaround: just copy image to new file with new name, as cache checks filesystem only
                    if (!origin.save(targetPath, "PNG")) {
                        qDebug() << "Unable to save transformed image with key " << targetKey << " to file " << targetPath;
                        emitter.on_next(false);
                        emitter.on_completed();
                        return;
                    }
                }

                QImage target(targetSize, QImage::Format_RGBA8888);
                //                std::shared_ptr<QPixmap> target = std::make_shared<QPixmap>(targetSize);

                std::shared_ptr<QPainter> painter = std::make_shared<QPainter>(&target);
                painter->setRenderHint(QPainter::Antialiasing, true);
                //                painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
                //                painter->setRenderHint(QPainter::LosslessImageRendering, true);
                painter->setBackgroundMode(Qt::BGMode::TransparentMode);
                //                painter->setCompositionMode(QPainter::CompositionMode_Source);

                painter->setCompositionMode(QPainter::CompositionMode_Clear);
                painter->fillRect(targetRect, QColor(0, 0, 0, 0));
                //                painter->fillRect(targetRect, QColor(0, 0, 0, 0));
                if (that.m_circular) {

                    QPainterPath path;
                    path.addEllipse(targetRect);
                    painter->setClipPath(path);
                }

                painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
                if (targetSize != origin.size()) {
                    painter->drawPixmap(0, 0, that.m_width, that.m_height, origin.scaled(that.m_width, that.m_height, that.m_ratio, that.m_mode));
                } else {
                    painter->drawPixmap(targetRect, origin);
                }

                qDebug() << "Origin image size: " << origin.size();
                qDebug() << "Origin image is null: " << origin.isNull();
                qDebug() << "Transformed image size: " << target.size();
                qDebug() << "Transformed image is null: " << target.isNull();

                if (!target.save(targetPath, "PNG")) {
                    qDebug() << "Unable to save transformed image with key " << targetKey << " to file " << targetPath;
                    emitter.on_next(false);
                    emitter.on_completed();
                    return;
                }

                emitter.on_next(true);
                emitter.on_completed();
            });
        }
    };

    class RequestOptions {
    private:
        QString url;
        QString key;
        std::shared_ptr<ImageTransformer> m_transformer;

    public:
        RequestOptions(QString url, QString key)
            : url(std::move(url)),
              key(std::move(key)) {
        }

        bool exist() {
            if (m_transformer) {
                //                qDebug() << "Imager: check for existent image by key " << m_transformer->createKey();
                return Imager::get().exist(m_transformer->createKey());
            }
            return Imager::get().exist(key);
        }

        RequestOptions& transform(const std::function<void(std::shared_ptr<ImageTransformer>)>& config) {
            m_transformer = std::make_shared<ImageTransformer>(key);
            config(m_transformer);
            return *this;
        }

        const QPixmap* get() {
            if (m_transformer != nullptr) {
                return Imager::get().load(m_transformer->createKey());
            } else {
                return Imager::get().load(key);
            }
        }

        void download(std::function<void(const QPixmap*)> onSuccess, const std::function<void(QString)>& onError = nullptr) {
            // if source image not exist
            if (!Imager::get().exist(key)) {
                auto imageLoader = Imager::get().loadImage(url, key);

                if (m_transformer) {
                    imageLoader.map([this](bool loadRes) {
                        if (!loadRes) {
                            return rxcpp::observable<>::just(loadRes).as_dynamic();
                        }
                        return m_transformer->transform();
                    });
                }

                RequestOptions that = *this;
                imageLoader
                    .subscribe_on(RxQt::get().ioThread())
                    .observe_on(RxQt::get().uiThread())
                    .subscribe(
                        [onSuccess, onError, that](bool res) {
                            if (res) {
                                if (that.m_transformer) {
                                    onSuccess(Imager::get().load(that.m_transformer->createKey()));
                                } else {
                                    onSuccess(Imager::get().load(that.key));
                                }
                            } else {
                                onError("Unknown error");
                            }
                        },
                        [onError](std::exception_ptr eptr) {
                            onError(miledger::utils::getError(eptr));
                        });
            }
            // if not exist only transformed image
            else if (m_transformer != nullptr && !Imager::get().exist(m_transformer->createKey())) {
                RequestOptions that = *this;
                m_transformer->transform()
                    .subscribe_on(RxQt::get().ioThread())
                    .observe_on(RxQt::get().uiThread())
                    .subscribe(
                        [onSuccess, onError, that](bool res) {
                            if (res) {
                                if (that.m_transformer != nullptr) {
                                    onSuccess(Imager::get().load(that.m_transformer->createKey()));
                                } else {
                                    onSuccess(Imager::get().load(that.key));
                                }
                            } else {
                                if (onError) {
                                    onError("Unknown error");
                                }
                            }
                        },
                        [onError](std::exception_ptr eptr) {
                            onError(miledger::utils::getError(eptr));
                        });
            }
            // if exist something
            else {
                if (m_transformer != nullptr) {
                    onSuccess(Imager::get().load(m_transformer->createKey()));
                } else {
                    onSuccess(Imager::get().load(key));
                }
            }
        }
    };

    static Imager& get() {
        static Imager ic;
        return ic;
    }

    QString keyForCoin(const minter::explorer::coin_item& coin) {
        return QString("coin_avatar_%1").arg(QString::fromStdString(coin.symbol));
    }

    QString keyForCoin(const QString& coinSymbol) {
        return QString("coin_avatar_%1").arg(coinSymbol);
    }

    //    QFile fileFromKey(const QString& key) {
    //        return QFile(filePathForKey(key));
    //    }

    bool exist(const minter::explorer::coin_item& coin, bool preload = true) {
        return exist(keyForCoin(coin), preload);
    }

    bool exist(const QString& key, bool preload = true) {
        bool contains;
        {
            std::lock_guard<std::mutex> lock(m_cacheLock);
            contains = m_inMemory.contains(key);
        }
        if (contains) {
            return true;
        } else {
            QFile image(filePathForKey(key));
            if (!image.exists()) {
                ;
                return false;
            }
            if (preload) {
                this->preload(key);
            }
        }
        return true;
    }

    QString filePathForKey(const minter::explorer::coin_item& coin) {
        return filePathForKey(keyForCoin(coin));
    }

    QString filePathForKey(const QString& key) {
        return QString("%1%2%3").arg(m_cacheDir, QDir::separator(), key);
    }

    bool preload(const minter::explorer::coin_item& coin) {
        return preload(keyForCoin(coin));
    }

    bool preload(const QString& key) {

        bool containsAndNotNull;
        {
            std::lock_guard<std::mutex> lock(m_cacheLock);
            containsAndNotNull = m_inMemory.contains(key) && !m_inMemory[key]->isNull();
        }
        if (containsAndNotNull) {
            return true;
        }

        QString imagePath = filePathForKey(key);

        if (!QFile::exists(imagePath)) {
            qDebug() << "File " << imagePath << " does not exists!";
            return false;
        }

        bool loaded;
        {
            std::lock_guard<std::mutex> lock(m_cacheLock);
            m_inMemory.insert(key, new QPixmap());
            loaded = m_inMemory.object(key)->load(imagePath);
        }
        if (!loaded) {
            qDebug() << "Unable to load imagePath from file " << imagePath << " to pixmap";
            return false;
        }
        return true;
    }

    ImageTransformer transform(const QString& key) {
        return ImageTransformer(key);
    }

    RequestOptions createRequest(QString url, QString key) {
        return RequestOptions(std::move(url), std::move(key));
    }

    const QPixmap* load(const QString& key) {

        {
            std::lock_guard<std::mutex> lock(m_cacheLock);
            if (m_inMemory.contains(key)) {
                return m_inMemory[key];
            }
        }

        if (!preload(key)) {
            return nullptr;
        }

        QPixmap* res;
        {
            std::lock_guard<std::mutex> lock(m_cacheLock);
            res = m_inMemory[key];
        }
        return res;
    }

    //    rxcpp::observable<QPixmap*> loadRequest() {
    //    }

    const QPixmap* load(const minter::explorer::coin_item& coin) {
        return load(keyForCoin(coin));
    }
};

#endif // MILEDGER_IMAGECACHE_H

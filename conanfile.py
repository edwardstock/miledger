import os

from conans import ConanFile, CMake


def get_version():
    with open(os.path.join(os.path.dirname(__file__), 'version'), 'r') as f:
        content = f.read()
        try:
            content = content.decode()
        except AttributeError:
            pass
        return content.strip()


class MiLedgerConan(ConanFile):
    name = "miledger"
    version = get_version()
    license = "GPL-3.0"
    author = "Eduard Maximovich edward.vstock@gmail.com"
    url = "https://github.com/edwardstock/miledger"
    description = "Minter C++ Ledger GUI wallet: connect with ledger minter application using this library"
    topics = ("minter", "minter-ledger", "ledger", "minter-network", "minter-blockchain", "blockchain")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        'network': {"testnet", "mainnet"}
    }
    default_options = {
        'network': 'testnet',
        'yaml-cpp:shared': False,
        'libsodium:shared': False,
        'toolbox:shared': False,
        'cpr:shared': False,
        'cpr:with_ssl': 'auto',
        'libcurl:shared': False,
        'libpng:shared': False,
        'minter_tx:shared': False,
        'fmt:shared': False,
        'qt:shared': True,
    }
    exports = "version"
    exports_sources = (
        "modules/*",
        "include/*",
        "cfg/*",
        "tests/*",
        "src/*",
        "libs/*",
        "ui/*",
        "resources/*",
        "CMakeLists.txt",
        "conanfile.py",
        "LICENSE",
        "README.md",
        "RELEASE_NOTES.md",
    )
    generators = "cmake"
    default_user = "minter"
    default_channel = "latest"

    requires = (
        'mhwallet/0.3.0@minter/latest',
        'minter_tx/2.0.3@minter/latest',
        'minter_api/0.2.0@minter/latest',
        'bigmath/1.0.7@edwardstock/latest',
        'yaml-cpp/0.6.3',
        'libsodium/1.0.18',
        'toolbox/3.2.3@edwardstock/latest',
        'nlohmann_json/3.9.1',
        'rxcpp/4.1.0',
        'cpr/1.6.2',
        'libcurl/7.69.1@',
        'libpng/1.6.37',
        'fmt/7.1.3',
        'qt/6.1.1',
        'restinio/0.6.9',
        'cxxopts/2.2.1',
    )

    build_requires = (
        # "gtest/1.10.0",
    )

    def source(self):
        if "CONAN_LOCAL" not in os.environ:
            self.run("rm -rf *")
            self.run("git clone --recursive https://github.com/edwardstock/miledger.git .")

    def configure(self):
        if self.settings.os != "Linux":
            self.options['qt'].qttools = True
            self.options['qt'].qtactiveqt = True
            self.options['qt'].qtdeclarative = True
            self.options['qt'].qtsvg = True

        if self.settings.compiler == "Visual Studio":
            del self.settings.compiler.runtime

    def build(self):
        cmake = CMake(self)
        opts = {
            'CMAKE_BUILD_TYPE': self.settings.build_type,
        }

        if self.options.get_safe('network', 'testnet') == 'testnet':
            opts['MINTER_TESTNET'] = 'On'

        cmake.configure(defs=opts)
        cmake.build()

    def package(self):
        self.copy("*", dst="include", src="include", keep_path=True)
        dir_types = ['bin', 'lib', 'Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel']
        file_types = ['lib', 'dll', 'dll.a', 'a', 'so', 'exp', 'pdb', 'ilk', 'dylib']

        for dirname in dir_types:
            for ftype in file_types:
                self.copy("*." + ftype, src=dirname, dst="lib", keep_path=False)

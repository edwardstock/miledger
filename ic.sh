#!/usr/bin/env bash
IN=$1
OUT=$2

inkscape --export-png=${OUT} \
--export-background-opacity=0 \
--export-width=128 \
--export-height=128 \
--without-gui ${IN}
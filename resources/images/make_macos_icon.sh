#!/bin/sh
set -e

# macos_icon/*.png is already named per iconutil's iconset convention
# (icon_WxH.png, icon_WxH@2x.png), so it can be copied straight in.
rm -Rf icon.iconset
mkdir icon.iconset
cp macos_icon/*.png icon.iconset/
rm -f icon.icns
iconutil -c icns icon.iconset
rm -Rf icon.iconset

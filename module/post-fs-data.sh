MODDIR=${0%/*}
mkdir -p /data/adb/camredirect
[ -f /data/adb/camredirect/config ] || echo "# package=cameraId" > /data/adb/camredirect/config
chmod 700 /data/adb/camredirect

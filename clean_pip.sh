rm python/latticex_rosetta.egg-info -rf
rm python/latticex/lib*.so -rf
rm python/latticex/lib128 -rf
rm dist -rf

rm build/*.linux-* -rf

# TODO: for user install latticex-rosetta with sudo please uncomment the  command blow
#sudo pip3 uninstall latticex-rosetta
if [ "$USER" == "root" ]; then
  sudo pip3 uninstall latticex-rosetta -y
else
  python3 -m pip uninstall latticex-rosetta -y # for the current user
fi

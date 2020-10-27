git submodule init
git submodule update
mkdir build
cd build
cmake ..
python2.7 ../glxw/glxw_gen.py --api khr --output ./glxw/
python2.7 ../glxw/glxw_gen.py --api egl --output ./glxw/
make

Maintainer: Nathan Casler

Image: CentOS 7

sudo yum install epel-release

sudo yum install gcc gcc-c++
##curl -O http://mirrors-usa.go-parts.com/gcc/releases/gcc-5.3.0/gcc-5.3.0.tar.gz
##tar -zxvf gcc-5.3.0.tar.gz
##mkdir gcc-build
##cd gcc-build
##../gcc-5.3.0/configure --disable-multilib --enable-languages=c,c++

sudo yum install git
sudo yum install cmake
# DOCKER CMDS
#sudo yum install docker
#sudo systemctl start docker.service
#sudo docker pull pdal/pdal

# GROUP Stuffs
sudo groupadd lidar
usermod -a -G lidar centos
sudo chown -R centos:lidar /srv/lidar

cd /srv/lidar


git clone https://github.com/pdal/pdal.git
cd pdal && mkdir build && cd build

git clone https://github.com/open-source-parsers/jsoncpp.git


ENV CC gcc
ENV CXX g++


# Install dependencies
# ZLIB
sudo yum install zlib-devel
#LibXML2
sudo yum install libxml2-devel

# SQLite
sudo yum install sqlite sqlite-devel
# Curl
sudo yum install curl curl-devel
# LibTIFF
sudo yum install libtiff libtiff-devel
# ProjLib
sudo yum install proj-devel

# LibGeoTIFF
#sudo yum install libgeotiff libgeotiff-devel
# GDAL
sudo yum install gdal gdal-devel
# GEOS
sudo yum install geos geos-devel
# BOOST
sudo yum install boost boost-devel

# python headers
sudo yum install python-devel
#PIP
sudo yum install python-pip
# Numpy
sudo pip install numpy

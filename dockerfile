
Maintainer: Nathan Casler

Image: CentOS 7

sudo yum install epel-release

sudo yum install gcc gcc-c++

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

git clone https://github.com/open-source-parsers/jsoancpp.git


ENV CC gcc
ENV CXX g++




sudo yum install epel-release gcc gcc-c++ git cmake docker zlib-devel libxml2-devel sqlite-devel curl-devel libtiff-devel proj-devel libgeotiff-devel gdal-devel geos-devel boost-devel python-devel jsoncpp-devel python-pip
# Numpy
sudo pip install numpy

LiDAR Modeling Engine
===

This library is intended to ease the manipulation and processing of large lidar
collections through a set of parallelized methods which interface between the
datasets and a HDF5 data store.

Dependencies
---

ZLIB: compression library
MPI: Message passing library
HDF5: Hierarchical Data Format(compiled with parallel support)
LibLAS: LAS data manipulation library
GEOS: Geometric Topology library
Proj.4: Projection transformation library
GDAL: Geograpic Data Abstraction Library (compiled with external libgeotiff)
LibGeoTiff: Geographic TIFF driver

Architecture
---

*LME Datastore*: Central data repository for lidar derivatives and metadata, keeps
track of datasets and user access privileges. Needed to efficiently distribute
processing to system.

*LiDAR Region*: A class which represents a geographic domain with LiDAR
coverage. These regions will have administrators and user access privileges. The
region will be initialized with a directory that contains LiDAR datasets.

*Dataset*: A LiDAR dataset (typically LAS format) which is registered for
processing. 

*Derivative*: A gridded dataset created by interpolating a subset of LiDAR
points from an area. This derivative will be owned by the user that requested
the data.

Workflow
---

1. Initialize DataStore
2. Register a LiDAR region
3. Request subset
4. Download result(or view a TMS online)




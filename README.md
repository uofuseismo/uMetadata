
# Query Examples

The following examples assume the server is running non-secured on your host
machine on port 50000.  Additionally, you must have  
[grpcurl](https://github.com/fullstorydev/grpcurl) installed.  

To list all services

    grpcurl --plaintext --proto ../proto/station.proto localhost:50000 list

Assuming the station proto file exists in /path/to/station.proto you can fetch all
active stations from the server
 
    grpcurl --plaintext --proto /path/to/proto/station.proto localhost:50000 UMetadata.GRPC.Information.GetAllActiveStations



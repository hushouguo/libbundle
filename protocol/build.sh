../deps/protobuf/bin/protoc --cpp_out=. common/*.proto
cp common/entity.pb.h ../deps/bundle/include
../deps/protobuf/bin/protoc --cpp_out=. recordserver/*.proto

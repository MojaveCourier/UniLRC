#sudo apt install protobuf-compiler-grpc
cd project/src/proto
protoc --proto_path=. --grpc_out=. --plugin=protoc-gen-grpc=./../../../third_party/grpc/cmake/build/grpc_cpp_plugin coordinator.proto
protoc --proto_path=. --cpp_out=. coordinator.proto
protoc --proto_path=. --grpc_out=. --plugin=protoc-gen-grpc=./../../../third_party/grpc/cmake/build/grpc_cpp_plugin proxy.proto
protoc --proto_path=. --cpp_out=. proxy.proto
protoc --proto_path=. --grpc_out=. --plugin=protoc-gen-grpc=./../../../third_party/grpc/cmake/build/grpc_cpp_plugin datanode.proto
protoc --proto_path=. --cpp_out=. datanode.proto

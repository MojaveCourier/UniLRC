cd project/src/proto
../../third_party/grpc/bin/protoc --proto_path=. --grpc_out=. --plugin=protoc-gen-grpc=./../../third_party/grpc/bin/grpc_cpp_plugin coordinator.proto
../../third_party/grpc/bin/protoc --proto_path=. --cpp_out=. coordinator.proto
../../third_party/grpc/bin/protoc --proto_path=. --grpc_out=. --plugin=protoc-gen-grpc=./../../third_party/grpc/bin/grpc_cpp_plugin proxy.proto
../../third_party/grpc/bin/protoc --proto_path=. --cpp_out=. proxy.proto
../../third_party/grpc/bin/protoc --proto_path=. --grpc_out=. --plugin=protoc-gen-grpc=./../../third_party/grpc/bin/grpc_cpp_plugin datanode.proto
../../third_party/grpc/bin/protoc --proto_path=. --cpp_out=. datanode.proto
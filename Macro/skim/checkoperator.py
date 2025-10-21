import onnx

# Load the ONNX model
model = onnx.load("fixed_model.onnx")

# Check the operators used in the model
graph = model.graph
operators = set(node.op_type for node in graph.node)
print("Operators in the model:", operators)

# Check the domain of the operators
for opset in model.opset_import:
    print(f"Domain: {opset.domain}, Version: {opset.version}")

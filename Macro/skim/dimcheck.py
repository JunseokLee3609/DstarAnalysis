import onnx
from onnx import helper

# Load the ONNX model
model = onnx.load("XGBoost_Model_0324_27.onnx")

# Check model validity
onnx.checker.check_model(model)

# Access the graph
graph = model.graph

# Inspect inputs
for input in graph.input:
    print(f"Input name: {input.name}")
    tensor_type = input.type.tensor_type
    if tensor_type.HasField("shape"):
        shape = tensor_type.shape
        print("  Shape:")
        for dim in shape.dim:
            if dim.HasField("dim_value"):
                print(f"    Dim value: {dim.dim_value}")
            elif dim.HasField("dim_param"):
                print(f"    Dim param: {dim.dim_param}")
            else:
                print("    No dim_value or dim_param defined!")
    else:
        print("  No shape defined!")

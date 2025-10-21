import onnx
from onnx import helper

# Load the ONNX model
model = onnx.load("XGBoost_Model_0324_27.onnx")

# Access the graph
graph = model.graph

# Fix the input shape
for input in graph.input:
    if input.name == "input":
        tensor_type = input.type.tensor_type
        if tensor_type.HasField("shape"):
            shape = tensor_type.shape
            for i, dim in enumerate(shape.dim):
                if not dim.HasField("dim_value") and not dim.HasField("dim_param"):
                    dim.dim_value = 1  # Set the undefined dimension to 1 (e.g., batch size)
                    print(f"Set dimension {i} to dim_value: 1")
                else:
                    print(f"Dimension {i}: {dim.dim_value if dim.HasField('dim_value') else dim.dim_param}")

# Validate the model
onnx.checker.check_model(model)

# Save the fixed model
onnx.save(model, "fixed_model.onnx")
print("Fixed model saved as 'fixed_model.onnx'")

#include <onnxruntime_cxx_api.h>
#include <vector>
#include <iostream>

void onnxtest(const std::string& model_path) {
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "test");
    Ort::SessionOptions session_options;
    Ort::Session session(env, model_path.c_str(), session_options);

    // 입력 정보 가져오기
    Ort::AllocatorWithDefaultOptions allocator;
    auto input_name = session.GetInputNameAllocated(0, allocator);
    std::vector<int64_t> input_shape = {1, 19}; // 입력 shape: [1, 19]
    std::vector<float> input_data(19, 1.0f); // 예시 입력 데이터

    // 입력 텐서 생성
    Ort::MemoryInfo memory_info("Cpu", OrtDeviceAllocator, 0, OrtMemTypeDefault);
    auto input_tensor = Ort::Value::CreateTensor<float>(
        memory_info, input_data.data(), input_data.size(),
        input_shape.data(), input_shape.size());

    // 출력 이름
    auto output_name = session.GetOutputNameAllocated(0, allocator);

    // 추론 실행
    std::vector<const char*> input_names = {input_name.get()};
    std::vector<const char*> output_names = {output_name.get()};
    std::vector<Ort::Value> output_tensors;
    session.Run(Ort::RunOptions{nullptr}, input_names.data(), &input_tensor, 1,
                output_names.data(), output_tensors.data(), 1);

    // 결과 처리
    float* output_data = output_tensors[0].GetTensorMutableData<float>();
    std::cout << "Output: " << output_data[0] << std::endl;
}

int main() {
    run_inference("xgboost_model.onnx");
    return 0;
}

#include "task_tinyml.h"

// Globals, for the convenience of one-shot setup.
namespace
{
    tflite::ErrorReporter *error_reporter = nullptr;
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    TfLiteTensor *input = nullptr;
    TfLiteTensor *output = nullptr;
    constexpr int kTensorArenaSize = 16 * 1024; // Adjust size based on your model
    uint8_t tensor_arena[kTensorArenaSize];
} // namespace

void setupTinyML()
{
    Serial.println("TensorFlow Lite Init....");
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(env_model_data); // g_model_data is from model_data.h
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        error_reporter->Report("Model provided is schema version %d, not equal to supported version %d.",
                               model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        error_reporter->Report("AllocateTensors() failed");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    Serial.println("TensorFlow Lite Micro initialized on ESP32.");
}

void predict(float *input_data, float *output_data)
{
    // Copy input data to the model's input tensor
    for (size_t i = 0; i < 2; i++)
    {
        input->data.f[i] = input_data[i];
    }

    // Run inference
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk)
    {
        error_reporter->Report("Invoke failed.");
        return;
    }

    // Copy output data from the model's output tensor
    for (size_t i = 0; i < 3; i++)
    {
        output_data[i] = output->data.f[i];
    }
}

void tiny_ml_task(void *pvParameters)
{
    for (;;)
    {
        // 1. Đọc cảm biến (Code cũ của bạn)
        float t = g_sensorData->temperature;
        float h = g_sensorData->humidity;

        // 2. CHÈN ĐOẠN CODE DỰ ĐOÁN VÀO ĐÂY
        float input[2] = {t, h};
        float prediction[3];
        predict(input, prediction);

        int predicted_class = 0;
        float max_prob = prediction[0];
        for (int i = 1; i < 3; i++)
        {
            if (prediction[i] > max_prob)
            {
                max_prob = prediction[i];
                predicted_class = i;
            }
        }

        Serial.print("AI Result: ");
        Serial.println(predicted_class);

        // 3. Logic Semaphore điều khiển đèn (Task 1, 2)
        if (predicted_class == 2)
        {
            // Code gửi tín hiệu nguy hiểm
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
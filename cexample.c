/* Example wrapper API we would implement. */
Detector *model = detector_load("rfdetr_nano.onnx");

int count = detector_predict(
    model, rgb_image, width, height, boxes, capacity
);

detector_free(model);
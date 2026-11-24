# Zybo project requirements — discussion draft

## Functional requirements

### UC1: Object detection using the ARM processor
The user uploads an image, and the system returns bounding boxes, class labels and confidence scores using ARM-only inference.

- **Test 1:** Process reference images and compare outputs with the original Python model.
- **Test 2:** Reject corrupt or unsupported images without crashing.
- **Test 3:** Process multiple images consecutively without stale results or increasing memory usage.

### UC2: Object detection using ARM + FPGA IP
The system processes the image using an FPGA accelerator for selected operations, with the remaining operations running on ARM.

- **Test 1:** Compare the IP’s output and the complete model’s output with their software references.
- **Test 2:** Verify that the IP actually executes and measure acceleration, including data transfers and synchronization.
- **Test 3:** Handle an IP timeout without hanging, and recover for subsequent requests.

### Image input
- Support user-uploaded images.
- Support images stored on an SD card.
- Support newly captured images from an agreed source.
- Define how to handle images arriving faster than the system can process them.

## Non-functional requirements

### Performance
- The accelerated implementation should reduce processing time compared with ARM-only execution.
- **5 seconds per image is a stretch target**, not a mandatory requirement yet.
- Compare both modes using the same inputs, model and numerical configuration.
- Measure processing time from a complete image being available in board RAM until detections are ready.
- Include preprocessing, inference, FPGA overhead and postprocessing.
- Report upload time and model initialization separately.

### Memory and FPGA resources
- Linux, the application, model and buffers must fit within the Zybo’s 512 MiB DDR without swap.
- The complete FPGA design must fit the XC7Z010 and meet timing at the selected clock.

### Agreement with the Python model
- Use identical weights, input resolution and preprocessing.
- For FP32, initially test with `atol=1e-4` and `rtol=1e-4`.
- Validate this tolerance on a representative set of images.
- If we use quantization, agree on a separate numerical tolerance and acceptable detection-accuracy loss.

## Points still to agree
- Does “live images” mean images sent from a PC or an attached camera?
- Which image formats and maximum dimensions should we support?
- Should we reject incoming images when busy, or keep only the newest pending image?

## Current baseline
Nano at 256×256 FP32 runs on ARM in approximately 25.3 seconds per inference. This was tested with preloaded tensors; image input handling and FPGA acceleration are still to be implemented.
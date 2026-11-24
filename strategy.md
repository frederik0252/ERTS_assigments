**Strategy means putting alternative ways of performing the same task behind a common interface, so the application can use any of them without changing its workflow.**

For our project, the task is **running inference**. ARM-only execution and ARM+FPGA execution would be two strategies.

**1. Define a family of algorithms**

Identify different ways to accomplish the same job:

- **ARM-only:** execute the model on the ARM processor.
- **ARM+FPGA:** execute most of the model on ARM and selected operations in an FPGA IP core.

Both accept an input tensor and produce model outputs.

Preprocessing and inference would **not** be two strategies for this interface because they perform different jobs.

**2. Encapsulate each implementation**

Put each implementation’s execution details inside its own component:

```text
ArmOnlyBackend
    Owns the CPU inference session
    Executes inference on ARM

ArmFpgaBackend — planned implementation
    Owns the hybrid inference session
    Dispatches selected operations to the FPGA
    Handles the required hardware interaction
```

The rest of the application should not need to manage those details.

**3. Make the implementations interchangeable**

Give both implementations the same interface:

```cpp
class InferenceBackend {
public:
    virtual ~InferenceBackend() = default;

    virtual ModelOutputs infer(const InputTensor& input) = 0;
};
```

This interface promises:

> Give me an input tensor, and I will return the model’s outputs.

Both backends must follow the same contract:

- Compatible input shapes and data types.
- The same preprocessing assumptions.
- Compatible output shapes and meaning.
- Agreement within the project’s numerical tolerances.

Having methods with the same name is not sufficient; their behaviour must satisfy the shared contract.

**4. Let the implementation vary independently from its client**

The *client* is the code that uses the strategy. In our project, that could be a `Detector`:

```cpp
class Detector {
public:
    explicit Detector(InferenceBackend& backend)
        : backend_(backend) {}

    Detections detect(const Image& image) {
        auto input = preprocess(image);
        auto outputs = backend_.infer(input);
        return postprocess(outputs);
    }

private:
    InferenceBackend& backend_;
};
```

This is a design sketch. The image, tensor and processing types are omitted.

The key line is:

```cpp
auto outputs = backend_.infer(input);
```

`Detector` delegates inference to its selected backend. Its overall workflow stays the same:

```text
Image → Preprocessing → Selected inference strategy → Postprocessing → Detections
```

The setup code chooses the strategy:

```cpp
ArmOnlyBackend backend;
Detector detector(backend);
```

Once the accelerated backend is implemented, the alternative setup would be:

```cpp
ArmFpgaBackend backend;
Detector detector(backend);
```

The `Detector` implementation does not need to change. In this example, the backend must remain alive for as long as the detector uses it.

**Pattern roles in our project**

| Strategy-pattern role | Project component |
|---|---|
| Strategy interface | `InferenceBackend` |
| Concrete strategy | `ArmOnlyBackend` |
| Alternative concrete strategy | `ArmFpgaBackend` |
| Context that uses the strategy | `Detector` |
| Code that selects the strategy | Application setup/configuration |

**Practical implications**

We can choose the strategy once at startup. Switching during execution is optional.

This structure lets us develop and compare the ARM-only and accelerated implementations while keeping preprocessing, postprocessing and application behaviour consistent.

Strategy provides the software separation. The actual performance improvement comes from the FPGA implementation and its integration.
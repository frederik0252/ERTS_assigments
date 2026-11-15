# ERTS Assignment 3

Firas Harbo Saleh's C++17 implementation of ERTS Assignment 3, developed with
assistance from a large reasoning model.

## Current implementation

The context forwards events to singleton states. The implemented startup flow
is PowerOnSelfTest -> Initializing -> Operational/Ready, with manually supplied
SelftestOk and Initialized events. SelfTestFailed(errorNo) enters Failure and
displays the error. Failure accepts Restart or Exit; Ready inherits Operational
Restart. Invalid events print a message and leave state and context data
unchanged. Per-system data lives in the context, not in singleton states.

An interactive console displays the actual state after each event. Configuration
supports Configure, ConfigX and ConfigurationEnded. RealTimeLoop enters Mode1,
changes between Mode1 and Mode2, responds to eventX for the current mode, and
returns to Ready on Stop. Restart from any Operational substate reruns self-test.
Reentering RealTimeLoop starts in Mode1, as in the supplied initial transition.

Actions such as systemSelftest, readConfigurationInfo, PerformConfigurationX,
run and responseM1eventX/responseM2eventX are simulated log messages; no physical
hardware self-test, configuration read, or application workload is implemented.
The accompanying report is provided separately. The UML diagrams and a code
walkthrough are included below. Zybo deployment is optional and has not been
performed.

The brief's `Initalized` spelling is normalized to `Initialized`. Mode-change
operations are named `chMode1` and `chMode2` consistently. `eventY` is declared
because it appears in the brief's class diagram; no behavior is specified for
it in the rendered state diagrams, so its default is to ignore it.

## Build and run

Requires CMake 3.16 or newer, a C++17 compiler and the platform's thread library.
Run these commands from the repository root.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/erts_console
```

Example commands: `SelftestOk`, `Initialized`, `Restart`, `SelfTestFailed 42`,
and `Exit`. Names are case sensitive. `help` lists commands; `quit` closes the
console from any state without sending the Failure-only Exit event.

Tests cover manual startup, failed self-test, invalid events preserving context
data, error display, restart, termination, configuration, both modes, mode
reentry, and isolation of two contexts sharing singleton states. The mode tests
also verify that a handler actually writes output from a different thread, that
the caller sees the resulting mode before returning, that concurrent callers
complete safely, and that the Request completion wrapper propagates exceptions
through its future.

## Active Object execution

The RealTimeLoop singleton is the state interface to a per-context Active Object.
The context owns the active mode and worker, so singleton states contain no
mutable application data. Every chMode1/chMode2 call in RealTimeLoop allocates
a new ModeChangeRequest capturing the source mode and its corresponding handler.
Requests are owned by a FIFO queue and deleted automatically after execution.

The worker invokes the captured source state's handler. For example, a request
for chMode2 in Mode1 calls Mode1::chMode2, which changes the context to Mode2.
Requesting chMode2 while already in Mode2 runs the inherited ignored-event
handler on that worker. Outside RealTimeLoop, mode events are ignored directly.

The caller waits on a future, making the console's state display deterministic.
An event mutex serializes public events, keeping Stop/Restart from overtaking a
pending mode request. A separate state mutex protects state and output access;
it is released before waiting so the worker can acquire it. State callbacks
reenter context helpers, so that state mutex is recursive. The worker drains
its queue and joins before the context's data is destroyed. Users must not call
methods during context destruction or reenter the context from its output stream.

Example session

```text
SelftestOk
Initialized
Configure
ConfigX
ConfigurationEnded
Start
eventX
chMode2
eventX
chMode1
Stop
Restart
SelfTestFailed 42
Start
Exit
```

## Code walkthrough

### Follow one ordinary event

After SelftestOk and Initialized, `state_` refers to the Ready singleton.
The console calls `EmbeddedSystemX::Start()`. That method calls `dispatch()`,
which invokes the virtual Start handler on the current state. The handler is
therefore `Ready::Start()`. It logs the run action and calls
`context.transitionTo(RealTimeLoop::instance())`.

`transitionTo()` updates the context's `state_` pointer and runs the new state's
onEntry method. RealTimeLoop::onEntry sets the context's `mode_` pointer to
Mode1::instance(). The console then displays Operational/RealTimeLoop/Mode1.
The state objects receive the context by reference so they can request these
updates through its private helper methods.

The corresponding implementation is in [EmbeddedSystemX.cpp](src/EmbeddedSystemX.cpp)
and [OperationalStates.cpp](src/OperationalStates.cpp).

### Identify the State pattern

State declares virtual handlers for the public events. Accepted events are
overridden by the concrete states. For example, Failure overrides Restart and
Exit. Its inherited Start handler prints that the event was ignored. The public
context interface stays the same as its current state changes.

Ready, Configuration and RealTimeLoop inherit Operational::Restart. Mode1 and
Mode2 inherit directly from State. The context keeps the enclosing RealTimeLoop
state active while forwarding mode events to its separate active mode. This
implements the state hierarchy shown in the behavioral diagram.

The common defaults are in [State.cpp](src/State.cpp), with the startup and
Operational declarations in [StartupStates.hpp](include/StartupStates.hpp).

### Identify the Singleton pattern

Each concrete state provides instance(), which returns a function-local static
object. Calls reuse that object. Leaf-state constructors are private, while
Operational's constructor is protected to allow its substates to derive from it.
The State base deletes copying, which also prevents copying its derived states.

The context stores its current state, current mode, error number, name and
version. Two contexts can share Failure::instance() and still display different
error numbers because each Failure handler reads the context passed to it.

Singleton definitions are in [StartupStates.cpp](src/StartupStates.cpp) and
[OperationalStates.cpp](src/OperationalStates.cpp).

### Follow a mode change

When mode_ refers to Mode1 and the console sends chMode2, the top-level
RealTimeLoop handler asks the context to queue the change. Its Active Object
allocates a ModeChangeRequest that captures this context, the Mode1 singleton
and the virtual chMode2 handler. The FIFO queue owns the request until the
worker takes ownership and executes it.

On the worker thread, Request::execute calls ModeChangeRequest::invoke, which
calls context.executeModeRequest. That operation invokes Mode1::chMode2 on
the captured source state. Mode1 asks the context to select Mode2. The request
fulfills its promise, allowing the waiting public chMode2 call to return.
The console then displays the resulting mode. The worker deletes the request
when it finishes processing it.

The event mutex stays locked until that call finishes, serializing public
events through completion.
The state mutex is released while the caller waits, giving the worker access
to the context. The sequence diagram shows both execution threads.

Request creation and worker execution are in
[RealTimeLoopActiveObject.cpp](src/RealTimeLoopActiveObject.cpp).

## UML diagrams

These diagrams describe the implemented desktop application. PlantUML sources
are editable, SVG files scale for document use, and PNG files provide previews.

| View | Diagram | Editable source |
| --- | --- | --- |
| Use case | [Control EmbeddedSystemX](docs/uml/01-use-case.svg) | [PlantUML](docs/uml/01-use-case.puml) |
| State behavior | [Hierarchy and transitions](docs/uml/02-state-machine.svg) | [PlantUML](docs/uml/02-state-machine.puml) |
| Logical view | [State and Singleton classes](docs/uml/03-state-classes.svg) | [PlantUML](docs/uml/03-state-classes.puml) |
| Active Object structure | [Ownership and requests](docs/uml/04-active-object-classes.svg) | [PlantUML](docs/uml/04-active-object-classes.puml) |
| Sequence | [chMode2 from Mode1](docs/uml/05-mode-change-sequence.svg) | [PlantUML](docs/uml/05-mode-change-sequence.puml) |

The class diagrams show selected operations. Their arrows follow the C++
inheritance and ownership relationships. The state diagram shows the behavioral
nesting of Operational and RealTimeLoop. The sequence diagram shows the caller
waiting while the worker processes a mode-change request.

To regenerate SVG and PNG files, run `bash scripts/render-uml.sh`. This optional
step requires Bash, Java 17 or newer, Graphviz (`dot`), and curl for the first
PlantUML download. The script downloads PlantUML 1.2026.10 into `.tools` when
needed. It locates Java on PATH or through JAVA_HOME. Set ERTS_JAVA or
ERTS_PLANTUML_JAR to use explicit local paths. The supplied diagrams can be
viewed without regenerating them; none of these tools is needed to build the
C++ application.

## Decisions

1. Invalid events print a message and leave the current state unchanged.
   The common default handlers implement this policy, avoiding duplication
   in each concrete state.
2. Self-test and initialization results are entered manually through the
   console so both success and failure paths can be exercised.
3. Mode changes execute on the worker, but the calling operation waits for
   completion before the console displays the resulting state.

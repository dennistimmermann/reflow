# IScreen base class — deferred idea

For a future refactor once there are multiple run screens: a lightweight vtable-based base
lets `ui::go_to()` call `hide()`/`show()` generically without a big switch.

```cpp
struct IScreen {
    virtual void show() = 0;
    virtual void hide() {}
    virtual ~IScreen() = default;
};
```

Each screen is a static singleton inheriting `IScreen`. `go_to()` just keeps a
`IScreen* current` and calls `current->hide()` then `next->show()`.

Revisit when Reflow run screen and Simple run screen are both implemented and the
switch in `go_to()` gets unwieldy.

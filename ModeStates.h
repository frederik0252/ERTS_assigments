#pragma once

class RealTimeLoop;   // forward declaration

class ModeState {
public:
    virtual ~ModeState() = default;
    virtual void chMode1(RealTimeLoop& loop);
    virtual void chMode2(RealTimeLoop& loop);
    virtual void eventX(RealTimeLoop& loop);
    virtual const char* name() const = 0;
};

class Mode1 : public ModeState {
public:
    static Mode1& getInstance();
    void chMode2(RealTimeLoop& loop) override;
    void eventX(RealTimeLoop& loop) override;      

    const char* name() const override;
private:
    Mode1() = default;
    Mode1(const Mode1&) = delete;
    Mode1& operator=(const Mode1&) = delete;
};

class Mode2 : public ModeState {
public:
    static Mode2& getInstance();
    void chMode1(RealTimeLoop& loop) override;
    void eventX(RealTimeLoop& loop) override;      
    const char* name() const override;
private:
    Mode2() = default;
    Mode2(const Mode2&) = delete;
    Mode2& operator=(const Mode2&) = delete;
};

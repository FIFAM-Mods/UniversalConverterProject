#pragma once

class Settings {
    bool mExtendLoansLimit = false;
public:
    bool getExtendLoansLimit();
    void setExtendLoansLimit(bool set);

    void Read();
    static Settings &GetInstance();
private:
    Settings();
};

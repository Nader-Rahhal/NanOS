namespace drivers::rtc {
class RTC {
private:
    RTC() = default;
    ~RTC() = default;
    
public:
    static RTC& getInstance() {
        static RTC instance;
        return instance;
    }

    void init() noexcept {
        ///
    }

    const char* get_date() noexcept {
        return "01/01/1970";
    }

    const char* get_time() noexcept {
        return "00:00:00";
    }
};
}
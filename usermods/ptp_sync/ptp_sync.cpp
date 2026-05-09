#include "wled.h"

class PtpSyncUsermod : public Usermod
{
private:
    static const char _name[];
    static const char _enabled[];
    static const char _backend[];
    static const char _status[];
    static const char _mode[];
    static const char _leader[];

    bool enabled = false;
    bool initDone = false;

    String backendMode = F("ftm");

    bool backendKnown() const
    {
        return backendMode.equalsIgnoreCase(F("ftm")) || backendMode.equalsIgnoreCase(F("tsf"));
    }

    String statusSummary() const
    {
        if (!enabled)
            return F("disabled");
        if (!WLEDNetwork.isConnected())
            return F("waiting-network");
        if (!backendKnown())
            return F("unsupported-backend");
        return backendMode + F("-scaffold");
    }

    String modeSummary() const
    {
        if (!enabled)
            return F("off");
        if (!backendKnown())
            return F("unknown");
        return backendMode.equalsIgnoreCase(F("ftm")) ? F("leader") : F("follower");
    }

    String leaderSummary() const
    {
        if (!enabled)
            return F("none");
        if (!backendKnown())
            return F("unknown");
        return backendMode.equalsIgnoreCase(F("ftm")) ? F("self") : F("AP");
    }

public:
    void setup() override
    {
        initDone = true;
    }

    void connected() override
    {
    }

    void loop() override
    {
        if (!initDone)
            return;
    }

    void addToJsonInfo(JsonObject &root) override
    {
        JsonObject user = root["u"];
        if (user.isNull())
            user = root.createNestedObject("u");

        JsonObject ptp = user[FPSTR(_name)];
        if (ptp.isNull()) ptp = user.createNestedObject(FPSTR(_name));

        ptp.createNestedArray(FPSTR(_status)).add(statusSummary());
        ptp.createNestedArray(FPSTR(_mode)).add(modeSummary());
        ptp.createNestedArray(FPSTR(_leader)).add(leaderSummary());
        ptp.createNestedArray(F("backend")).add(backendMode);
    }

    void addToJsonState(JsonObject &root) override
    {
        if (!initDone)
            return;

        JsonObject top = root[FPSTR(_name)];
        if (top.isNull())
            top = root.createNestedObject(FPSTR(_name));

        top[FPSTR(_enabled)] = enabled;
        top[FPSTR(_backend)] = backendMode;
        top[FPSTR(_status)] = statusSummary();
        top[FPSTR(_mode)] = modeSummary();
        top[FPSTR(_leader)] = leaderSummary();
        top["running"] = enabled && WLEDNetwork.isConnected();
        top["sourceValid"] = enabled && backendKnown() && WLEDNetwork.isConnected();
    }

    void readFromJsonState(JsonObject &root) override
    {
        JsonObject top = root[FPSTR(_name)];
        if (top.isNull())
            return;

        bool wasEnabled = enabled;

        getJsonValue(top[FPSTR(_enabled)], enabled);
        getJsonValue(top[FPSTR(_backend)], backendMode);

        if (!wasEnabled && enabled && !backendKnown())
        {
            backendMode = F("ftm");
        }
    }

    void addToConfig(JsonObject &root) override
    {
        JsonObject top = root.createNestedObject(FPSTR(_name));
        top[FPSTR(_enabled)] = enabled;
        top[FPSTR(_backend)] = backendMode;
    }

    bool readFromConfig(JsonObject &root) override
    {
        JsonObject top = root[FPSTR(_name)];
        bool configComplete = !top.isNull();

        configComplete &= getJsonValue(top[FPSTR(_enabled)], enabled);
        configComplete &= getJsonValue(top[FPSTR(_backend)], backendMode);

        if (!backendKnown())
            backendMode = F("ftm");
        return configComplete;
    }

    void appendConfigData() override
    {
        oappend(F("addInfo('")); oappend(String(FPSTR(_name)).c_str()); oappend(F(":enabled',1,'<i>turn Wi-Fi PTP sync on or off</i><br><i class=\"warn\">changes require reboot</i>');"));
        oappend(F("addInfo('")); oappend(String(FPSTR(_name)).c_str()); oappend(F(":status',1,'<i>")); oappend(statusSummary()); oappend(F("</i>');"));
        oappend(F("addInfo('")); oappend(String(FPSTR(_name)).c_str()); oappend(F(":mode',1,'<i>")); oappend(modeSummary()); oappend(F("</i>');"));
        oappend(F("addInfo('")); oappend(String(FPSTR(_name)).c_str()); oappend(F(":leader',1,'<i>")); oappend(leaderSummary()); oappend(F("</i>');"));
    }

    void onUpdateBegin(bool init) override
    {
        (void)init;
    }
};

const char PtpSyncUsermod::_name[] PROGMEM = "WiFi PTP Sync";
const char PtpSyncUsermod::_enabled[] PROGMEM = "enabled";
const char PtpSyncUsermod::_backend[] PROGMEM = "backend";
const char PtpSyncUsermod::_status[] PROGMEM = "status";
const char PtpSyncUsermod::_mode[] PROGMEM = "mode";
const char PtpSyncUsermod::_leader[] PROGMEM = "leader";

static PtpSyncUsermod ptp_sync;
REGISTER_USERMOD(ptp_sync);
#include "pch.h"
#include "_version.h"
#include "TTFSDK.h"
#include "Updater.h"

namespace Updater {

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    bool QueryServerForUpdates(rapidjson::Document* document)
    {
        CURL* curl;
        CURLcode res;

        curl_mime* form = NULL;
        curl_mimepart* field = NULL;

        std::string readBuffer;

        curl = curl_easy_init();
        if (curl) {
            form = curl_mime_init(curl);

            field = curl_mime_addpart(form);
            curl_mime_name(field, "ver");
            curl_mime_data(field, BME_VERSION, CURL_ZERO_TERMINATED);

            field = curl_mime_addpart(form);
            curl_mime_name(field, "env");
            curl_mime_data(field, GetBMEChannel().c_str(), CURL_ZERO_TERMINATED);

            curl_easy_setopt(curl, CURLOPT_URL, "https://bme.titanfall.top/backend/update_check.php");
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "BME/" BME_VERSION);
            curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK)
                return false;
            //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

            curl_easy_cleanup(curl);
            curl_mime_free(form);
        }

        if (readBuffer.empty()) return false;

        if (document->Parse(readBuffer.c_str()).HasParseError()) return false;

        if (!document->IsObject() && document->IsBool()) return false;
        return true;

    }

    //////////////////

    double updaterNowDownloaded = 0;
    double updaterTotalToDownload = 1;
    bool isUpdaterDownloadInProgress = false;
    bool isUpdaterDownloadCancelled = false;
    double updaterDownloadProgress = 0.0f;

    size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
        size_t written = fwrite(ptr, size, nmemb, stream);
        return written;
    }

    int progress_func(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
    {
        if (Updater::isUpdaterDownloadCancelled)
            return 1;
        updaterDownloadProgress = NowDownloaded / TotalToDownload;
        updaterNowDownloaded = NowDownloaded;
        updaterTotalToDownload = TotalToDownload;
        return 0;
    }

    std::wstring Updater::SaveUpdaterFile(std::string url)
    {
        isUpdaterDownloadInProgress = true;
        isUpdaterDownloadCancelled = false;
        updaterDownloadProgress = 0.0;

        CURL* curl;
        FILE* fp;
        CURLcode res;

        wchar_t temp[MAX_PATH];
        if (!GetTempPathW(MAX_PATH - 16, temp)) return L""; //return NULL;
        std::wstring outfilename{ temp };
        outfilename += L"bme_updater.exe";

        spdlog::info("Gonna save update to: {}", Util::Narrow(outfilename));

        curl = curl_easy_init();
        if (curl) {
            fp = _wfopen(outfilename.c_str(), L"wb");
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
            curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);
            res = curl_easy_perform(curl);
            isUpdaterDownloadInProgress = false;

            if (res != CURLE_OK) return L""; //return NULL;

            curl_easy_cleanup(curl);
            fclose(fp);
        }

        return outfilename;
    }

    bool pendingUpdateLaunch = false;
    bool pendingUpdateLaunchMotdChange = false;
    bool updateInProcess = false;
    std::wstring updater_path;
    std::string params;
    bool isUpdaterLaunching = false;
    bool drawModalWillUpdaterLaunchAfterGameClose = false;

    void LaunchUpdater()
    {
        isUpdaterLaunching = true; // TODO: in case UI drawing causes crash, move this to the bottom?

        auto logger = spdlog::get("logger");
        RegisterApplicationRestart(GetCommandLineW(), 0);

        STARTUPINFOW si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        std::wstring params2 = Util::Widen(params);

        logger->info("Firing up BME updater...");
        logger->info("Path: {}", Util::Narrow(updater_path));
        logger->info("Params: {}", params);

        CreateProcessW(updater_path.c_str(), (LPWSTR)params2.c_str(), NULL, NULL, false, 0, NULL, NULL, &si, &pi);

        /*if (&SDK() != nullptr)
            FreeSDK();

        if (g_console)
            g_console.reset();*/
    }

    DWORD WINAPI DownloadAndApplyUpdate(LPVOID lpThreadParameter)
    {
        Sleep(1);
        auto logger = spdlog::get("logger");
        logger->info("Checking for updates...");

        rapidjson::Document d;
        bool result = QueryServerForUpdates(&d);
        if (result == false || !d.IsObject())
        {
            logger->info("No updates found.");
            return 0;
        }

        updateInProcess = true;
        logger->info("Found an update, downloading updater...");
        logger->info("Downloading from: {}", d["updater_url"].GetString());

        updater_path = SaveUpdaterFile(d["updater_url"].GetString());
        if (Updater::isUpdaterDownloadCancelled)
        {
            logger->error("Update cancelled by user.");
            return 0;
        }
        // TODO: poni�sze jako MsgBox?
        if (updater_path.empty())
        {
            logger->error("Found update, but could not save the file to user's temp directory. Check in with your antivirus software and create a whitelist entry.");
            return 0;
        }

        logger->info("Updater saved to: {}", Util::Narrow(updater_path));

        params = std::string(d["updater_params"].GetString());
        params = std::regex_replace(params, std::regex("\\$dir"), GetThisPath());

        logger->info("Updater command line params: {}", params);

        logger->info("====");
        logger->info("BME updater will be fired up after you close your game.");
        logger->info("If you wish to skip this update, you can set cvar in console: bme_skip_update 1");
        logger->info("Alternatively, if you wish to never check for updates in the future, you can use the command line option: -bmenoupdates");
        logger->info("====");

        pendingUpdateLaunch = true; // change, we will always launch updater after game close...
        pendingUpdateLaunchMotdChange = true;
        if (&SDK() != nullptr && SDK().runFrameHookCalled)
        {
            //pendingUpdateLaunch = true;
            //logger->info(_("Game is already launched, delaying update until it's quit."));
        }
        else
        {
            //LaunchUpdater();
            drawModalWillUpdaterLaunchAfterGameClose = true;
        }
        return 0;
    }

    void CheckForUpdates()
    {
        CreateThread(NULL, 0, DownloadAndApplyUpdate, NULL, 0, NULL);
    }

}
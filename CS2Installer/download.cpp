#include "download.hpp"
#include "globals.hpp"

#include <Windows.h>
#include <filesystem>

std::filesystem::path GetLocalAppData() {
    char* localAppData;
    size_t bufferSize = 0;
    errno_t result = _dupenv_s(&localAppData, &bufferSize, "LOCALAPPDATA");
    if (result != NULL || !localAppData) {
        printf("failed to get local appdata directory.\n");
        system("pause");
        exit(0);
    }

    std::filesystem::path localAppDataPath(static_cast<const char*>(localAppData));

    return localAppDataPath;
}

void Downloader::PrepareDownload() {
    /* General Data we need for Preparing */
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path localAppData = GetLocalAppData();
    const char* manifestNames[3] = { "730_2347770", "730_2347771", "730_2347779" };
    const char* depotKeys = "depot_keys.json";

    /* create manifest folder and download manifest files */
    std::filesystem::create_directory(currentPath / "manifestFiles");

    if (!Globals::usesNoManifests) {
        std::filesystem::remove_all("manifestFiles"); // delete the manifestFiles folder everytime we download so that we dont accidentally cause the game to not update

        int maxIndex = sizeof(manifestNames) / sizeof(manifestNames[0]);
        for (int downloadIndex = 0; downloadIndex < maxIndex; ++downloadIndex) { /* download our manifest files */
            std::string downloadLink = "https://github.com/CS2-OOF-LV/CS2Installer-Dependencies/raw/main/";
            downloadLink += manifestNames[downloadIndex];

            std::string downloadPath = "manifestFiles/";
            downloadPath += manifestNames[downloadIndex];

            HRESULT downloadedManifest = URLDownloadToFileA(NULL, downloadLink.c_str(), downloadPath.c_str(), NULL, NULL);
            if (downloadedManifest != S_OK) {
                printf("failed to download manifest file.\n");
                printf("please report this to Nebel: %i\n", downloadedManifest);
                system("pause");
                exit(0);
            }
        }
    }

    /* Create steamctl folders and import depot keys */
    std::filesystem::path steamctlDirectory = localAppData / "steamctl";
    std::filesystem::create_directory(steamctlDirectory);
    std::filesystem::create_directory(steamctlDirectory / "steamctl");

    if (!Globals::usesNoManifests) {
        std::string downloadLink = "https://github.com/CS2-OOF-LV/CS2Installer-Dependencies/raw/main/";
        downloadLink += depotKeys;

        std::string downloadPath = localAppData.string();
        downloadPath += "\\steamctl\\steamctl\\";
        downloadPath += depotKeys;

        HRESULT downloadedKeys = URLDownloadToFileA(NULL, downloadLink.c_str(), downloadPath.c_str(), NULL, NULL);
        if (downloadedKeys != S_OK) {
            printf("failed to download depot keys.\n");
            printf("please report this to Nebel: %i\n", downloadedKeys);
            system("pause");
            exit(0);
        }
    }
}

void Downloader::DownloadCS2() {
    /* General Data we need for Downloading */
    std::filesystem::path currentPath = std::filesystem::current_path();
    const char* manifestNames[3] = { "730_2347770", "730_2347771", "730_2347779" };

    std::string stringPath = currentPath.string();
    std::filesystem::current_path(stringPath.c_str());

    /* Download CS2 using steams python library named steamctl */
    int maxIndex = sizeof(manifestNames) / sizeof(manifestNames[0]);
    for (int downloadIndex = 0; downloadIndex < (maxIndex); ++downloadIndex) {
        std::string manifestPath = "manifestFiles\\";
        manifestPath += manifestNames[downloadIndex];

        /* execute steamctl command using our manifests */
        std::string executeDownload = currentPath.string() + "\\";
        executeDownload += "python-3.11.4-embed-amd64\\python.exe -m steamctl depot download -f ";
        executeDownload += manifestPath;
        executeDownload += " --skip-licenses --skip-login";
        //printf("command executed -> %s\n", executeDownload.c_str());
        system(executeDownload.c_str());
    }
}

void Downloader::DownloadMods() {
    std::filesystem::path currentPath = std::filesystem::current_path();
    const char* githubPaths[9] = {
        "https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/game/csgo_mods/pak01_000.vpk",
        "https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/game/csgo_mods/pak01_dir.vpk",
        "https://raw.githubusercontent.com/CS2-OOF-LV/CS2-Client/main/Mod%20Loading%20Files/game/csgo/gameinfo.gi",
        "https://raw.githubusercontent.com/CS2-OOF-LV/CS2-Client/main/Mod%20Loading%20Files/game/csgo/scripts/vscripts/banList.lua",
        "https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/Start%20Game%20(English)%20-%20DEBUG.bat",
        "https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/Start%20Game%20(English).bat",
        "https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/Start%20Server.bat",
        "https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/Workshop%20Tools%20-%20RAYTRACING.bat",
        "https://github.com/CS2-OOF-LV/CS2-Client/raw/main/Mod%20Loading%20Files/Workshop%20Tools.bat" };

    const char* filePaths[9] = {
        "game\\csgo_mods\\pak01_000.vpk",
        "game\\csgo_mods\\pak01_dir.vpk",
        "game\\csgo\\gameinfo.gi",
        "game\\csgo\\scripts\\vscripts\\banList.lua",
        "Start Game (English) - DEBUG.bat",
        "Start Game (English).bat",
        "Start Server.bat",
        "Workshop Tools - RAYTRACING.bat",
        "Workshop Tools.bat" };

    /* Create mods folder */
    std::filesystem::create_directory(currentPath / "game" / "csgo_mods");

    /* remove the gameinfo.gi from csgo folder so that we can replace it with ours */
    std::filesystem::path filePath = "game/csgo/gameinfo.gi";
    if (std::filesystem::exists(filePath)) {
        std::filesystem::remove(filePath);
    }

    /* download files to specific directories */
    int maxIndex = sizeof(filePaths) / sizeof(filePaths[0]);
    for (int downloadIndex = 0; downloadIndex < maxIndex; ++downloadIndex) {
        std::filesystem::path downloadFilePath = currentPath / filePaths[downloadIndex];
        std::string downloadPath = downloadFilePath.string();
        //printf("%s\n", downloadPath.c_str());
        HRESULT downloadedKeys = URLDownloadToFileA(NULL, githubPaths[downloadIndex], downloadPath.c_str(), NULL, NULL);
        if (downloadedKeys != S_OK) {
            printf("failed to download depot keys.\n");
            printf("please report this to Nebel: %i\n", downloadedKeys);
            system("pause");
            exit(0);
        }
    }
}
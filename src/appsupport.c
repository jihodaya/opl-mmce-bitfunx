#include "include/opl.h"
#include "include/lang.h"
#include "include/gui.h"
#include "include/appsupport.h"
#include "include/themes.h"
#include "include/system.h"
#include "include/ioman.h"
#include "include/util.h"

#include "include/bdmsupport.h"
#include "include/ethsupport.h"
#include "include/hddsupport.h"
#include "include/mmcesupport.h"

#include <elf-loader.h>
#include <zlib.h>

static int appForceUpdate = 1;
static int appItemCount = 0;
static time_t appPopsModified;

static config_set_t *configApps;
static app_info_t *appsList;

struct app_info_linked
{
    struct app_info_linked *next;
    app_info_t app;
};

// forward declaration
static item_list_t appItemList;

static void appFreeList(void);

#define MMCE_POPS_DIR "mmce0:/POPS/"
#define POPSTARTER_SELECTOR "mass:/POPS/XX."

static int appAddMmceVcds(struct app_info_linked **apps)
{
    item_list_t *mmce = mmceGetObject(1);
    if (!mmce || strncmp(mmce->itemGetPrefix(mmce), "mmce0:/", 7) != 0)
        return 0;

    DIR *dir = opendir(MMCE_POPS_DIR);
    if (!dir)
        return 0;

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        size_t length = strlen(entry->d_name);
        if (entry->d_type == DT_DIR || length <= 4 || length > APP_ARGV1_MAX ||
            length - 4 + sizeof("[PS1] ") > APP_TITLE_MAX + 1 ||
            strcasecmp(entry->d_name + length - 4, ".vcd") != 0)
            continue;

        struct app_info_linked *item = calloc(1, sizeof(*item));
        if (!item)
            break;
        snprintf(item->app.title, sizeof(item->app.title), "[PS1] %.*s", (int)length - 4, entry->d_name);
        snprintf(item->app.path, sizeof(item->app.path), "%s", MMCE_POPS_DIR);
        snprintf(item->app.argv1, sizeof(item->app.argv1), "%s", entry->d_name);
        item->app.mmceVcd = 1;
        item->next = *apps;
        *apps = item;
        count++;
    }
    closedir(dir);
    return count;
}

static time_t appMmcePopsModified(void)
{
    struct stat info;
    item_list_t *mmce = mmceGetObject(1);
    if (mmce && !strncmp(mmce->itemGetPrefix(mmce), "mmce0:/", 7) && stat(MMCE_POPS_DIR, &info) == 0)
        return info.st_mtime;
    return 0;
}

static struct config_value_t *appGetConfigValue(int id)
{
    struct config_value_t *cur = configApps->head;

    while (id--) {
        cur = cur->next;
    }

    return cur;
}

static char *appGetELFName(char *name)
{
    // Looking for the ELF name
    char *pos = strrchr(name, '/');
    if (!pos)
        pos = strrchr(name, ':');
    if (pos) {
        return pos + 1;
    }

    return name;
}

static float appGetELFSize(char *path)
{
    int fd, size;
    float bytesInMiB = 1048576.0f;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        LOG("Failed to open APP %s\n", path);
        return 0.0f;
    }

    size = getFileSize(fd);
    close(fd);

    // Return size in MiB
    return (size / bytesInMiB);
}

static char *appGetBoot(char *device, int max, char *path)
{
    char *pos, *filenamesep;

    // Looking for the boot device & filename from the path
    pos = strrchr(path, ':');
    if (pos != NULL) {
        int len = (int)(pos + 1 - path);
        if (len + 1 > max)
            len = max - 1;
        strncpy(device, path, len);
        device[len] = '\0';
    }

    filenamesep = strchr(path, '/');
    if (filenamesep != NULL)
        return filenamesep + 1;

    if (pos) {
        return pos + 1;
    }

    return path;
}

void appInit(item_list_t *itemList)
{
    LOG("APPSUPPORT Init\n");
    appForceUpdate = 1;
    configGetInt(configGetByType(CONFIG_OPL), "app_frames_delay", &appItemList.delay);
    if (configApps)
        configFree(configApps);
    configApps = oplGetLegacyAppsConfig();
    appItemList.enabled = 1;
}

item_list_t *appGetObject(int initOnly)
{
    if (initOnly && !appItemList.enabled)
        return NULL;
    return &appItemList;
}

void appRequestRefresh(void)
{
    appForceUpdate = 1;
}

static int appNeedsUpdate(item_list_t *itemList)
{
    int update;

    update = 0;
    if (appForceUpdate) {
        appForceUpdate = 0;
        update = 1;
    }
    if (oplShouldAppsUpdate())
        update = 1;

    time_t modified = appMmcePopsModified();
    if (modified != appPopsModified) {
        appPopsModified = modified;
        update = 1;
    }

    if (update) {
        if (configApps)
            configFree(configApps);
        configApps = oplGetLegacyAppsConfig();
    }

    return update;
}

static int addAppsLegacyList(struct app_info_linked **appsLinkedList)
{
    struct config_value_t *cur;
    struct app_info_linked *app;
    int count;

    configClear(configApps);
    configRead(configApps);

    count = 0;
    cur = configApps->head;
    while (cur != NULL) {
        app = malloc(sizeof(*app));
        if (app == NULL) {
            LOG("APPSUPPORT unable to allocate memory.\n");
            break;
        }
        app->next = *appsLinkedList;
        *appsLinkedList = app;

        strncpy(app->app.title, cur->key, APP_TITLE_MAX + 1);
        app->app.title[APP_TITLE_MAX] = '\0';

        // Split the boot filename from the path.
        const char *elfname = appGetELFName(cur->val);
        if (elfname != cur->val) {
            strncpy(app->app.boot, elfname, APP_BOOT_MAX + 1);
            app->app.boot[APP_BOOT_MAX] = '\0';

            int pathlen = (int)(elfname - cur->val) - 1;
            if (cur->val[pathlen] == ':') // Discard only '/'.
                pathlen++;
            if (pathlen > APP_PATH_MAX)
                pathlen = APP_PATH_MAX;
            strncpy(app->app.path, cur->val, pathlen);
            app->app.path[pathlen] = '\0';
        } else {
            // Cannot split boot filename from the path, somehow.
            strncpy(app->app.boot, cur->val, APP_BOOT_MAX + 1);
            app->app.boot[APP_BOOT_MAX] = '\0';
            strncpy(app->app.path, cur->val, APP_PATH_MAX + 1);
            app->app.path[APP_BOOT_MAX] = '\0';
        }

        app->app.legacy = 1;
        app->app.mmceVcd = 0;
        count++;
        cur = cur->next;
    }

    return count;
}

static int appScanCallback(const char *path, config_set_t *appConfig, void *arg)
{
    struct app_info_linked **appsLinkedList = (struct app_info_linked **)arg;
    struct app_info_linked *app;
    const char *title, *boot, *argv1;

    if (configGetStr(appConfig, APP_CONFIG_TITLE, &title) != 0 && configGetStr(appConfig, APP_CONFIG_BOOT, &boot) != 0) {
        app = malloc(sizeof(*app));
        if (app == NULL) {
            LOG("APPSUPPORT unable to allocate memory.\n");
            return -1;
        }
        app->next = *appsLinkedList;
        *appsLinkedList = app;

        strncpy(app->app.title, title, APP_TITLE_MAX + 1);
        app->app.title[APP_TITLE_MAX] = '\0';
        strncpy(app->app.boot, boot, APP_BOOT_MAX + 1);
        app->app.boot[APP_BOOT_MAX] = '\0';
        strncpy(app->app.path, path, APP_PATH_MAX + 1);
        app->app.path[APP_PATH_MAX] = '\0';
        if (configGetStr(appConfig, APP_CONFIG_ARGV1, &argv1) != 0) {
            strncpy(app->app.argv1, argv1, APP_ARGV1_MAX + 1);
            app->app.argv1[APP_ARGV1_MAX] = '\0';
        } else
            app->app.argv1[0] = '\0';
        app->app.legacy = 0;
        app->app.mmceVcd = 0;
        return 0;
    } else {
        LOG("APPSUPPORT item has no boot/title.\n");
        return 1;
    }

    return -1;
}

static int appUpdateItemList(item_list_t *itemList)
{
    struct app_info_linked *appsLinkedList, *appNext;

    appFreeList();

    appsLinkedList = NULL;

    // Get legacy apps list first, so it is possible to use appGetConfigValue(id).
    appItemCount += addAppsLegacyList(&appsLinkedList);

    // Scan devices for apps.
    appItemCount += oplScanApps(&appScanCallback, &appsLinkedList);
    appItemCount += appAddMmceVcds(&appsLinkedList);

    // Generate apps list
    if (appItemCount > 0) {
        appsList = malloc(appItemCount * sizeof(app_info_t));

        if (appsList != NULL) {
            int i;
            for (i = 0; appsLinkedList != NULL; i++) { // appsLinkedList contains items in reverse order.
                memcpy(&appsList[appItemCount - i - 1], &appsLinkedList->app, sizeof(app_info_t));

                appNext = appsLinkedList->next;
                free(appsLinkedList);
                appsLinkedList = appNext;
            }
        } else {
            LOG("APPSUPPORT unable to allocate memory.\n");
            appItemCount = 0;
            while (appsLinkedList != NULL) {
                appNext = appsLinkedList->next;
                free(appsLinkedList);
                appsLinkedList = appNext;
            }
        }
    }

    LOG("APPSUPPORT %d apps loaded\n", appItemCount);

    return appItemCount;
}

static void appFreeList(void)
{
    free(appsList);
    appsList = NULL;
    appItemCount = 0;
}

static int appGetItemCount(item_list_t *itemList)
{
    return appItemCount;
}

static char *appGetItemName(item_list_t *itemList, int id)
{
    return appsList[id].title;
}

static int appGetItemNameLength(item_list_t *itemList, int id)
{
    return CONFIG_KEY_NAME_LEN;
}

/* appGetItemStartup() is called to get the startup path for display & for the art assets.
   The path is used immediately, before a subsequent call to appGetItemStartup(). */
static char *appGetItemStartup(item_list_t *itemList, int id)
{
    if (appsList[id].mmceVcd) {
        static char artKey[sizeof(MMCE_POPS_DIR) + APP_ARGV1_MAX];
        snprintf(artKey, sizeof(artKey), "%s%s", MMCE_POPS_DIR, appsList[id].argv1);
        return artKey;
    }
    if (appsList[id].legacy) {
        struct config_value_t *cur = appGetConfigValue(id);
        return appGetELFName(cur->val);
    } else {
        return appsList[id].boot;
    }
}

static void appDeleteItem(item_list_t *itemList, int id)
{
    if (appsList[id].mmceVcd)
        return;
    if (appsList[id].legacy) {
        struct config_value_t *cur = appGetConfigValue(id);
        unlink(cur->val);
        cur->key[0] = '\0';
        configApps->modified = 1;
        configWrite(configApps);
    } else {
        sysDeleteFolder(appsList[id].path);
    }

    appForceUpdate = 1;
}

static void appRenameItem(item_list_t *itemList, int id, char *newName)
{
    char value[256];

    if (appsList[id].mmceVcd)
        return;
    if (appsList[id].legacy) {
        struct config_value_t *cur = appGetConfigValue(id);

        strncpy(value, cur->val, sizeof(value));
        configRemoveKey(configApps, cur->key);
        configSetStr(configApps, newName, value);
        configWrite(configApps);
    } else {
        config_set_t *appConfig;

        snprintf(value, sizeof(value), "%s/%s", appsList[id].path, APP_TITLE_CONFIG_FILE);

        appConfig = configAlloc(0, NULL, value);
        if (appConfig != NULL) {
            configRead(appConfig);
            configSetStr(appConfig, APP_CONFIG_TITLE, newName);
            configWrite(appConfig);

            configFree(appConfig);
        }
    }

    appForceUpdate = 1;
}

static int appFileExists(const char *path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return 0;
    int size = getFileSize(fd);
    close(fd);
    return size > 0;
}

static int appMatchPopsDriver(const char *path, int expectedSize, u32 expectedCRC)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return 0;

    u8 buffer[1024];
    u32 crc = crc32(0, Z_NULL, 0);
    int count, total = 0;
    while ((count = read(fd, buffer, sizeof(buffer))) > 0) {
        total += count;
        if (total > expectedSize)
            break;
        crc = crc32(crc, buffer, count);
    }
    close(fd);
    return count == 0 && total == expectedSize && crc == expectedCRC;
}

static void appLaunchMmceVcd(const char *vcd, const char *existingElf)
{
    char path[256], selector[256];
    size_t length = strlen(vcd);
    if (length <= 4 || strcasecmp(vcd + length - 4, ".VCD") != 0 ||
        strcspn(vcd, "/\\:") != length) {
        guiMsgBox(_l(_STR_MMCE_POPS_VCD_MISSING), 0, NULL);
        return;
    }

    int selectorLength = snprintf(selector, sizeof(selector), POPSTARTER_SELECTOR "%.*s.ELF", (int)length - 4, vcd);
    if (selectorLength >= sizeof(selector) || snprintf(path, sizeof(path), MMCE_POPS_DIR "%s", vcd) >= sizeof(path)) {
        guiMsgBox(_l(_STR_MMCE_POPS_NAME_TOO_LONG), 0, NULL);
        return;
    }

    ioBlockOps(1);
    const char *error = NULL;

    if (!appFileExists(path)) {
        error = _l(_STR_MMCE_POPS_VCD_MISSING);
        goto failed;
    }

    const char *elf = MMCE_POPS_DIR "POPSTARTER.ELF";
    if (!appFileExists(elf)) {
        if (existingElf == NULL || !appFileExists(existingElf)) {
            error = _l(_STR_MMCE_POPS_ELF_MISSING);
            goto failed;
        }
        elf = existingElf;
    }

    if (!appFileExists(MMCE_POPS_DIR "POPS_IOX.PAK")) {
        error = _l(_STR_MMCE_POPS_FILES_MISSING);
        goto failed;
    }

    const char *usbd = "mc0:/POPSTARTER/usbd.irx";
    const char *usbhdfsd = "mc0:/POPSTARTER/usbhdfsd.irx";
    if (!appFileExists(usbd) || !appFileExists(usbhdfsd)) {
        error = _l(_STR_MMCE_POPS_DRIVERS_MISSING);
        goto failed;
    }

    if (strlen(elf) + strlen(selector) + 2 > 256) {
        error = _l(_STR_MMCE_POPS_NAME_TOO_LONG);
        goto failed;
    }

    // Identify the slot 1 driver from ps2-mmce/mmceman's POPS4MMCE_OPEN_TEST_BUILD001.zip.
    if (!appMatchPopsDriver(usbd, 11841, 0xba780630) || !appMatchPopsDriver(usbhdfsd, 19733, 0x82149d87)) {
        ioBlockOps(0);
        if (!guiMsgBox(_l(_STR_MMCE_POPS_DRIVERS_UNKNOWN), 1, NULL))
            return;
    }

    deinit(UNMOUNT_EXCEPTION, MMCE_MODE);
    sysLaunchPopstarter(elf, selector);
    return;

failed:
    ioBlockOps(0);
    guiMsgBox(error, 0, NULL);
}

static void appLaunchItem(item_list_t *itemList, int id, config_set_t *configSet)
{
    int fd;
    char filename[256];
    const char *argv1;

    if (appsList[id].mmceVcd) {
        char vcd[APP_ARGV1_MAX + 1];
        strcpy(vcd, appsList[id].argv1);
        appLaunchMmceVcd(vcd, NULL);
        return;
    }

    // Retrieve configuration set by appGetConfig()
    if (!configGetStrCopy(configSet, CONFIG_ITEM_STARTUP, filename, sizeof(filename)))
        return;

    if (!strncmp(filename, "mmce0:", 6) || !strncmp(filename, "mmce1:", 6)) {
        const char *name = appGetELFName(filename);
        size_t length = strlen(name);
        if (length > 7 && !strncasecmp(name, "XX.", 3) && !strcasecmp(name + length - 4, ".elf")) {
            if (filename[4] != '0') {
                guiMsgBox(_l(_STR_MMCE_POPS_SLOT_ONE), 0, NULL);
                return;
            }
            char vcd[256];
            snprintf(vcd, sizeof(vcd), "%.*s.VCD", (int)length - 7, name + 3);
            appLaunchMmceVcd(vcd, filename);
            return;
        }
    }

    // If no device number is specified use mass? to auto find device number
    const char *oldPrefix = "mass:";
    const char *newPrefix = "mass?:";

    if (strncmp(filename, oldPrefix, strlen(oldPrefix)) == 0) {
        size_t oldPrefixLen = strlen(oldPrefix);
        size_t newPrefixLen = strlen(newPrefix);
        memmove(filename + newPrefixLen, filename + oldPrefixLen, strlen(filename) - oldPrefixLen + 1);

        memcpy(filename, newPrefix, newPrefixLen);
    }

    // If legacy apps state mass? find the first connected mass device with the corresponding filename and set the unit number for launch.
    if (!strncmp("mass?", filename, 5)) {
        for (int i = 0; i < BDM_MODE4; i++) {
            filename[4] = i + '0';
            fd = open(filename, O_RDONLY);
            if (fd >= 0) {
                close(fd);
                break;
            }
        }
    }

    fd = open(filename, O_RDONLY);
    if (fd >= 0) {
        int mode, argc = 0;
        char partition[128];
        char *argv[1];
        close(fd);

        strcpy(partition, "");

        // To keep the necessary device accessible, we will assume the mode that owns the device which contains the file to boot.
        mode = oplPath2Mode(filename);
        if (mode < 0)
            mode = APP_MODE; // Legacy apps mode on memory card (mc?:/*)

        if (mode == HDD_MODE)
            snprintf(partition, sizeof(partition), "%s:", gOPLPart);

        if (configGetStr(configSet, CONFIG_ITEM_ALTSTARTUP, &argv1) != 0) {
            argv[0] = (char *)argv1;
            argc = 1;
        }

        deinit(UNMOUNT_EXCEPTION, mode);
        LoadELFFromFileWithPartition(filename, partition, argc, argv);
    } else
        guiMsgBox(_l(_STR_ERR_FILE_INVALID), 0, NULL);
}

static config_set_t *appGetConfig(item_list_t *itemList, int id)
{
    config_set_t *config;
    char tmp[8];

    if (appsList[id].mmceVcd) {
        char path[256];
        snprintf(path, sizeof(path), MMCE_POPS_DIR "%s", appsList[id].argv1);
        config = configAlloc(0, NULL, NULL);
        configSetStr(config, CONFIG_ITEM_NAME, appsList[id].argv1);
        configSetStr(config, CONFIG_ITEM_LONGNAME, appsList[id].title);
        configSetStr(config, CONFIG_ITEM_STARTUP, path);
        configSetStr(config, CONFIG_ITEM_MEDIA, "PS1");
        configSetStr(config, CONFIG_ITEM_FORMAT, "VCD");
    } else if (appsList[id].legacy) {
        struct config_value_t *cur = appGetConfigValue(id);
        config = oplGetLegacyAppsInfo(appGetELFName(cur->val));
        configRead(config);

        configSetStr(config, CONFIG_ITEM_NAME, appGetELFName(cur->val));
        configSetStr(config, CONFIG_ITEM_LONGNAME, cur->key);
        configSetStr(config, CONFIG_ITEM_STARTUP, cur->val);
        configSetStr(config, CONFIG_ITEM_MEDIA, "APP");
        configSetStr(config, CONFIG_ITEM_FORMAT, "ELF");

        snprintf(tmp, sizeof(tmp), "%.2f", appGetELFSize(cur->val));
        configSetStr(config, CONFIG_ITEM_SIZE, tmp);
    } else {
        char path[256];
        snprintf(path, sizeof(path), "%s/%s", appsList[id].path, APP_TITLE_CONFIG_FILE);

        config = configAlloc(0, NULL, path);
        configRead(config); // Does not matter if the config file could be loaded or not.

        configSetStr(config, CONFIG_ITEM_NAME, appsList[id].boot);
        configSetStr(config, CONFIG_ITEM_LONGNAME, appsList[id].title);
        configSetStr(config, CONFIG_ITEM_ALTSTARTUP, appsList[id].argv1); // reuse AltStartup for argument 1
        snprintf(path, sizeof(path), "%s/%s", appsList[id].path, appsList[id].boot);
        configSetStr(config, CONFIG_ITEM_STARTUP, path);
        configSetStr(config, CONFIG_ITEM_MEDIA, "APP");
        configSetStr(config, CONFIG_ITEM_FORMAT, "ELF");

        snprintf(tmp, sizeof(tmp), "%.2f", appGetELFSize(path));
        configSetStr(config, CONFIG_ITEM_SIZE, tmp);
    }
    return config;
}

static int appGetImage(item_list_t *itemList, char *folder, int isRelative, char *value, char *suffix, GSTEXTURE *resultTex, short psm)
{
    char device[8] = "", *startup;

    if (isRelative && !strcmp(folder, "ART") && !strncmp(value, MMCE_POPS_DIR, sizeof(MMCE_POPS_DIR) - 1)) {
        item_list_t *mmce = mmceGetObject(1);
        const char *vcd = value + sizeof(MMCE_POPS_DIR) - 1;
        size_t length = strlen(vcd);
        if (mmce && length > 4 && !strcasecmp(vcd + length - 4, ".vcd") && length - 4 <= APP_ARGV1_MAX) {
            char name[APP_ARGV1_MAX + 1];
            memcpy(name, vcd, length - 4);
            name[length - 4] = '\0';
            return mmce->itemGetImage(mmce, folder, isRelative, name, suffix, resultTex, psm);
        }
    }

    startup = appGetBoot(device, sizeof(device), value);

    if (!strcmp(folder, "ART"))
        return oplGetAppImage(device, folder, isRelative, startup, suffix, resultTex, psm);
    else
        return oplGetAppImage(device, folder, isRelative, value, suffix, resultTex, psm);
}

static int appGetTextId(item_list_t *itemList)
{
    return _STR_APPS;
}

static int appGetIconId(item_list_t *itemList)
{
    return APP_ICON;
}

// This may be called, even if appInit() was not.
static void appCleanUp(item_list_t *itemList, int exception)
{
    if (appItemList.enabled) {
        LOG("APPSUPPORT CleanUp\n");

        appFreeList();
        if (configApps)
            configFree(configApps);
        configApps = NULL;
    }
}

static void appShutdown(item_list_t *itemList)
{
    appCleanUp(itemList, NO_EXCEPTION);
}

static item_list_t appItemList = {
    APP_MODE, -1, 0, MODE_FLAG_NO_COMPAT | MODE_FLAG_NO_UPDATE, MENU_MIN_INACTIVE_FRAMES, APP_MODE_UPDATE_DELAY, NULL, NULL, &appGetTextId, NULL, &appInit, &appNeedsUpdate, &appUpdateItemList,
    &appGetItemCount, NULL, &appGetItemName, &appGetItemNameLength, &appGetItemStartup, &appDeleteItem, &appRenameItem, &appLaunchItem,
    &appGetConfig, &appGetImage, &appCleanUp, &appShutdown, NULL, &appGetIconId};

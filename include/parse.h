#ifndef parse_h
#define parse_h

/* System Includes */
#include <string.h>
#include <stdlib.h>
/* Support Library */
#include "std.h"

#define MAX_NUMBER_OF_SETTINGS 512

/* Interface Definitions */
void populateSettingsWithFile(String filename);
void populateSettingsToFile(String filename);   /* for data storage */
void unPopulateSettings(void);
void commitChanges(void);

void* getSettingForKey(String key);
int getSettingForKeyIntValue(String key);
String getSettingForKeyStringValue(String key);
bool getSettingForKeyBoolValue(String key);
void printAllSettings(void);

/* Pointers to Functions.
 * Initialized in the initialize() function in std.h */
void* (*getValueForKey)(String key);
int (*getValueForKeyIntValue)(String key);
String (*getValueForKeyStringValue)(String key);
bool (*getValueForKeyBoolValue)(String key);

/* Global Structures */
typedef struct {
	String key[MAX_NUMBER_OF_SETTINGS];
	void *value[MAX_NUMBER_OF_SETTINGS];
	int count;
	int max_count;
    String toFile;
} extractedSettings;

/* Internal Function Definitions */
void initializeSettings(void);
int addSetting(String key, void *value);

/* Global Data Structures */
extractedSettings settings;

#endif

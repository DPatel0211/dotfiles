#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <libnotify/notify.h>
#include <math.h>

#define BATTERY_STATUS_FILE "/sys/class/power_supply/BAT0/status"
#define BATTERY_CAPACITY_FILE "/sys/class/power_supply/BAT0/capacity"
#define BATTERY_ENERGY_FILE "/sys/class/power_supply/BAT0/energy_now"
#define BATTERY_POWER_FILE "/sys/class/power_supply/BAT0/power_now"

#define STATE_DIR "/tmp/battery_monitor"
#define CHARGING_FLAG STATE_DIR "/charging"
#define FULL_FLAG STATE_DIR "/full"
#define CRITICAL_FLAG STATE_DIR "/critical"
#define CRITICAL_THRESHOLD 20

typedef enum {
    UNKNOWN,
    CHARGING,
    DISCHARGING,
    FULL,
    NOT_CHARGING
} battery_status_t;

int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

int create_flag_file(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    fclose(f);
    return 1;
}

int remove_flag_file(const char *path) {
    return unlink(path) == 0 || !file_exists(path);
}

int read_battery_capacity() {
    FILE *f = fopen(BATTERY_CAPACITY_FILE, "r");
    if (!f) return -1;
    int capacity;
    if (fscanf(f, "%d", &capacity) != 1) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return capacity;
}

int read_battery_energy_now() {
    FILE *f = fopen(BATTERY_ENERGY_FILE, "r");
    if (!f) return -1;
    int energy;
    if (fscanf(f, "%d", &energy) != 1) { fclose(f); return -1; }
    fclose(f);
    return energy;
}

int read_battery_power_now() {
    FILE *f = fopen(BATTERY_POWER_FILE, "r");
    if (!f) return -1;
    int power;
    if (fscanf(f, "%d", &power) != 1) { fclose(f); return -1; }
    fclose(f);
    return power;
}

battery_status_t read_battery_status() {
    FILE *f = fopen(BATTERY_STATUS_FILE, "r");
    if (!f) return UNKNOWN;
    
    char status[32];
    if (!fgets(status, sizeof(status), f)) {
        fclose(f);
        return UNKNOWN;
    }
    fclose(f);
    
    status[strcspn(status, "\n")] = 0;
    
    if (strcmp(status, "Charging") == 0) return CHARGING;
    if (strcmp(status, "Discharging") == 0) return DISCHARGING;
    if (strcmp(status, "Full") == 0) return FULL;
    if (strcmp(status, "Not charging") == 0) return NOT_CHARGING;
    
    return UNKNOWN;
}

void send_notification(const char *title, const char *body) {
    NotifyNotification *notification = notify_notification_new(title, body, NULL);
    if (notification) {
        notify_notification_set_timeout(notification, 5000);
        notify_notification_show(notification, NULL);
        g_object_unref(notification);
    }
}

void format_time_remaining(battery_status_t status, char *buffer, size_t size) {
    int energy = read_battery_energy_now();
    int power = read_battery_power_now();
    int capacity = read_battery_capacity();

    if (energy <= 0 || power <= 0 || capacity < 0) {
        snprintf(buffer, size, "Battery: %d%%", capacity >= 0 ? capacity : 0);
        return;
    }

    double hours = 0.0;

    if (status == DISCHARGING) {
        hours = (double)energy / power;
    } else if (status == CHARGING) {
        int energy_full = energy * 100 / capacity; // rough full energy estimate
        hours = (double)(energy_full - energy) / power;
    }

    // Ensure no negative values
    hours = fabs(hours);

    int hr = (int)hours;
    int min = (int)((hours - hr) * 60);

    if (status == DISCHARGING) {
        if (hr > 0)
            snprintf(buffer, size, "%d hr %d min remaining until empty", hr, min);
        else
            snprintf(buffer, size, "%d min remaining until empty", min);
    } else if (status == CHARGING) {
        if (hr > 0)
            snprintf(buffer, size, "%d hr %d min until full", hr, min);
        else
            snprintf(buffer, size, "%d min until full", min);
    } else {
        snprintf(buffer, size, "Battery: %d%%", capacity);
    }
}

void handle_charging_state(battery_status_t status, int capacity) {
    int charging_flag_exists = file_exists(CHARGING_FLAG);
    int full_flag_exists = file_exists(FULL_FLAG);
    int critical_flag_exists = file_exists(CRITICAL_FLAG);

    char message[64];
    format_time_remaining(status, message, sizeof(message));
    
    if (status == CHARGING) {
        if (!charging_flag_exists) {
            send_notification("Connected to Charging Device", message);
            create_flag_file(CHARGING_FLAG);
        }

        if (critical_flag_exists)
            remove_flag_file(CRITICAL_FLAG);

        if (capacity >= 100 && !full_flag_exists) {
            send_notification("Device is Fully Charged!", "Please unplug charging device");
            create_flag_file(FULL_FLAG);
        }

        if (capacity < 100 && full_flag_exists)
            remove_flag_file(FULL_FLAG);

    } else if (status == NOT_CHARGING || status == FULL) {
        if (!charging_flag_exists)
            create_flag_file(CHARGING_FLAG);

        if (critical_flag_exists)
            remove_flag_file(CRITICAL_FLAG);

        if (capacity >= 95 && !full_flag_exists) {
            send_notification("Device is Fully Charged!", "Please unplug charging device");
            create_flag_file(FULL_FLAG);
        }

        if (capacity < 95 && full_flag_exists)
            remove_flag_file(FULL_FLAG);

    } else if (status == DISCHARGING) {
        if (charging_flag_exists) {
            send_notification("Charging Device Disconnected", message);
            remove_flag_file(CHARGING_FLAG);
        }

        if (full_flag_exists)
            remove_flag_file(FULL_FLAG);

        if (capacity <= CRITICAL_THRESHOLD && capacity >= 0 && !critical_flag_exists) {
            send_notification("Critical Condition!", "Please connect to charging device");
            create_flag_file(CRITICAL_FLAG);
        }

        if (capacity > CRITICAL_THRESHOLD && critical_flag_exists)
            remove_flag_file(CRITICAL_FLAG);

    } else {
        if (charging_flag_exists) {
            send_notification("Charging Device Disconnected", message);
            remove_flag_file(CHARGING_FLAG);
        }

        if (full_flag_exists)
            remove_flag_file(FULL_FLAG);
    }
}

int main() {
    if (!notify_init("battery-monitor")) {
        fprintf(stderr, "Failed to initialize libnotify\n");
        return 1;
    }

    mkdir(STATE_DIR, 0755);
    printf("Battery monitor started. Polling every 10 seconds...\n");

    while (1) {
        battery_status_t status = read_battery_status();
        int capacity = read_battery_capacity();
        if (status != UNKNOWN && capacity >= 0) {
            handle_charging_state(status, capacity);
        } else {
            fprintf(stderr, "Warning: Could not read battery status/capacity\n");
        }
        sleep(10);
    }

    notify_uninit();
    return 0;
}

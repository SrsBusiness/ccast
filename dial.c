#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <upnp/upnp.h>
#include "list.h"
#include "discover.h"

int discovery_search_result(struct Upnp_Discovery *event);
int callback(Upnp_EventType etype, void *event, void *cookie);
void print_device(const struct chromecast_device *x);

UpnpClient_Handle handle = -1;

//const char CHROMECAST_DEVICE_TYPE[] = "urn:dial-multiscreen-org:service:dial:1";
const char CHROMECAST_DEVICE_TYPE[] = "urn:dial-multiscreen-org:device:dial:1";

/* TODO: create thread to periodically search */
int dial_discover() {
    UpnpSearchAsync(handle, 5, CHROMECAST_DEVICE_TYPE, NULL); 
    return 0;
}

int dial_init() {
    list_init(&devices);
    int status = UpnpInit(NULL, 0);
    UpnpRegisterClient(callback, NULL, &handle);
    return status;
}

int dial_finish() {
    return UpnpFinish();
}

int chromecast_matches_UUID(const void *_UUID, void *_device) {
    const char *UUID = _UUID; 
    const struct chromecast_device *device = _device;
    return strcmp(device->device_UUID, UUID) == 0;
}

int callback(Upnp_EventType etype, void *event, void *cookie) {
    /* cookie unused */
    switch (etype) {
        case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
            break;
        case UPNP_DISCOVERY_SEARCH_RESULT:
            return discovery_search_result(event);
        case UPNP_DISCOVERY_SEARCH_TIMEOUT:
            break;
        case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
            break;
        case UPNP_CONTROL_ACTION_COMPLETE:
            break;
        case UPNP_CONTROL_GET_VAR_COMPLETE:
            break;
        case UPNP_EVENT_RECEIVED:
            break;
        case UPNP_EVENT_SUBSCRIBE_COMPLETE:
            break;
        case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
            break;
        case UPNP_EVENT_RENEWAL_COMPLETE:
            break;
        case UPNP_EVENT_AUTORENEWAL_FAILED:
            break;
        case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
            break;
        case UPNP_EVENT_SUBSCRIPTION_REQUEST:
            break;
        case UPNP_CONTROL_GET_VAR_REQUEST:
            break;
        case UPNP_CONTROL_ACTION_REQUEST:
            break;
    }
    return 0;
}

/* 
 * supported properties are:
 * deviceType
 * friendlyName
 * manufacturer
 * modelName
 * UDN
 * serviceType
 * serviceId
 **/
const char *get_device_property(IXML_Document *desc, const char *prop) {
    IXML_NodeList *list = ixmlDocument_getElementsByTagName(desc, prop);
    int length = ixmlNodeList_length(list);
    if (length < 1) {
        fprintf(stderr, "Error obtaining device name\n");
    }
    IXML_Node *n = ixmlNodeList_item(list, 0);
    n = ixmlNode_getFirstChild(n);
    return ixmlNode_getNodeValue(n);
}

char *create_string_copy(const char *str) {
    int len = strlen(str) + 1;
    char *copy = malloc(len);
    memcpy(copy, str, len);
    return copy;
}

int discovery_search_result(struct Upnp_Discovery *event) {
    if (event->ErrCode != UPNP_E_SUCCESS) {
        fprintf(stderr, "Error in discovering device\n");
        exit(-1);
    }

    IXML_Document *desc = NULL;
    int ret = UpnpDownloadXmlDoc(event->Location, &desc);
    
    if (ret != UPNP_E_SUCCESS) {
        fprintf(stderr, "Error in obtaining device description\n");
        exit(-1);
    }
    
    const char *UUID = get_device_property(desc, "UDN");

    if (!list_contains(&devices, UUID, chromecast_matches_UUID)) {
        struct chromecast_device *device = malloc(sizeof(struct chromecast_device));
        device->addr = ((struct sockaddr_in *)&event->DestAddr)->sin_addr;     
        device->device_name = create_string_copy(get_device_property(desc, "friendlyName"));
        device->device_type = create_string_copy(get_device_property(desc, "deviceType"));
        device->device_UUID = create_string_copy(UUID);
        device->device_OS = create_string_copy(event->Os);
        device->device_manufacturer = create_string_copy(get_device_property(desc, "manufacturer"));
        device->device_model_name = create_string_copy(get_device_property(desc, "modelName"));
        device->service_type = create_string_copy(get_device_property(desc, "serviceType"));
        device->service_version = create_string_copy(event->ServiceVer);
        device->service_id = create_string_copy(get_device_property(desc, "serviceId"));
        list_add_sync(&devices, device);
        print_device(device);
    }
    ixmlDocument_free(desc);
    return 0;
}

void print_device(const struct chromecast_device *x) {
    printf("IP Address: %s\n", inet_ntoa(x->addr));
    printf("Device Name: %s\n", x->device_name);
    printf("Device Type: %s\n", x->device_type);
    printf("Device UUID: %s\n", x->device_UUID);
    printf("Device OS: %s\n", x->device_OS);
    printf("Device Manufacturer: %s\n", x->device_manufacturer);
    printf("Device Model Name: %s\n", x->device_model_name);
    printf("Service Type: %s\n", x->service_type);
    printf("Service Version: %s\n", x->service_version);
    printf("Service ID: %s\n", x->service_id);
}

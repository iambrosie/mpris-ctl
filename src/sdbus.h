/**
 * @author Marius Orcsik <marius@habarnam.ro>
 */

#include <stdio.h>
#include <dbus/dbus.h>

#define LOCAL_NAME                 "org.mpris.mprisctl"
#define MPRIS_PLAYER_NAMESPACE     "org.mpris.MediaPlayer2"
#define MPRIS_PLAYER_PATH          "/org/mpris/MediaPlayer2"
#define MPRIS_PLAYER_INTERFACE     "org.mpris.MediaPlayer2.Player"
#define MPRIS_METHOD_NEXT          "Next"
#define MPRIS_METHOD_PREVIOUS      "Previous"
#define MPRIS_METHOD_PLAY          "Play"
#define MPRIS_METHOD_PAUSE         "Pause"
#define MPRIS_METHOD_STOP          "Stop"
#define MPRIS_METHOD_PLAY_PAUSE    "PlayPause"

#define MPRIS_PNAME_PLAYBACKSTATUS "PlaybackStatus"
#define MPRIS_PNAME_CANCONTROL     "CanControl"
#define MPRIS_PNAME_CANGONEXT      "CanGoNext"
#define MPRIS_PNAME_CANGOPREVIOUS  "CanGoPrevious"
#define MPRIS_PNAME_CANPLAY        "CanPlay"
#define MPRIS_PNAME_CANPAUSE       "CanPause"
#define MPRIS_PNAME_CANSEEK        "CanSeek"
#define MPRIS_PNAME_SHUFFLE        "Shuffle"
#define MPRIS_PNAME_POSITION       "Position"
#define MPRIS_PNAME_VOLUME         "Volume"
#define MPRIS_PNAME_LOOPSTATUS     "LoopStatus"
#define MPRIS_PNAME_METADATA       "Metadata"

#define MPRIS_PROP_PLAYBACK_STATUS "PlaybackStatus"
#define MPRIS_PROP_METADATA        "Metadata"
#define MPRIS_ARG_PLAYER_IDENTITY  "Identity"

#define DBUS_DESTINATION           "org.freedesktop.DBus"
#define DBUS_PATH                  "/"
#define DBUS_INTERFACE             "org.freedesktop.DBus"
#define DBUS_PROPERTIES_INTERFACE  "org.freedesktop.DBus.Properties"
#define DBUS_METHOD_LIST_NAMES     "ListNames"
#define DBUS_METHOD_GET_ALL        "GetAll"
#define DBUS_METHOD_GET            "Get"

#define MPRIS_METADATA_BITRATE      "bitrate"
#define MPRIS_METADATA_ART_URL      "mpris:artUrl"
#define MPRIS_METADATA_LENGTH       "mpris:length"
#define MPRIS_METADATA_TRACKID      "mpris:trackid"
#define MPRIS_METADATA_ALBUM        "xesam:album"
#define MPRIS_METADATA_ALBUM_ARTIST "xesam:albumArtist"
#define MPRIS_METADATA_ARTIST       "xesam:artist"
#define MPRIS_METADATA_COMMENT      "xesam:comment"
#define MPRIS_METADATA_TITLE        "xesam:title"
#define MPRIS_METADATA_TRACK_NUMBER "xesam:trackNumber"
#define MPRIS_METADATA_URL          "xesam:url"
#define MPRIS_METADATA_YEAR         "year"

// The default timeout leads to hangs when calling
//   certain players which don't seem to reply to MPRIS methods
#define DBUS_CONNECTION_TIMEOUT     100 //ms

#define MAX_PROP_LENGTH             256

char* get_zero_string(size_t length);

typedef struct mpris_metadata {
    int track_number;
    int bitrate;
    int disc_number;
    int length; // mpris specific
    char* album_artist;
    char* composer;
    char* genre;
    char* artist;
    char* comment;
    char* track_id;
    char* album;
    char* content_created;
    char* title;
    char* url;
    char* art_url; //mpris specific

} mpris_metadata;

typedef struct mpris_properties {
    mpris_metadata* metadata;
    double volume;
    int64_t position;
    char* player_name;
    char* loop_status;
    char* playback_status;
    bool can_control;
    bool can_go_next;
    bool can_go_previous;
    bool can_play;
    bool can_pause;
    bool can_seek;
    bool shuffle;
} mpris_properties;

void mpris_metadata_init(mpris_metadata *m)
{
    if (NULL == m) { return; }
    m->track_number = 0;
    m->bitrate = 0;
    m->disc_number = 0;
    m->length = 0;
    m->track_id = 0;
    m->url = 0;
    m->art_url = 0;
    m->content_created = 0;
    m->album_artist = get_zero_string(MAX_PROP_LENGTH);
    if (NULL == m->album_artist) { return; }
    m->composer = get_zero_string(MAX_PROP_LENGTH);
    if (NULL == m->composer) { return; }
    m->genre = get_zero_string(MAX_PROP_LENGTH);
    if (NULL == m->genre) { return; }
    m->artist = get_zero_string(MAX_PROP_LENGTH);
    if (NULL == m->artist) { return; }
    m->comment = get_zero_string(2 * MAX_PROP_LENGTH);
    if (NULL == m->comment) { return; }
    m->album = get_zero_string(MAX_PROP_LENGTH);
    if (NULL == m->album) { return; }
    m->title = get_zero_string(MAX_PROP_LENGTH);
    printf("album artist: %p\n", m->album_artist);
    printf("composer: %p\n", m->composer);
    printf("genre: %p\n", m->genre);
    printf("artist: %p\n", m->artist);
    printf("comment: %p\n", m->comment);
    printf("album: %p\n", m->album);
    printf("title: %p\n", m->title);
}

void mpris_properties_init(mpris_properties *p)
{
    if (NULL == p) { return; }
    p->can_control = false;
    p->can_go_next = false;
    p->can_go_previous = false;
    p->can_play = false;
    p->can_pause = false;
    p->can_seek = false;
    p->shuffle = false;
    p->volume = 0;
    p->position = 0;
    p->player_name = get_zero_string(MAX_PROP_LENGTH);
    if (NULL == p->player_name) { return; }
    p->loop_status = get_zero_string(MAX_PROP_LENGTH);
    if (NULL == p->loop_status) { return; }
    p->playback_status = get_zero_string(MAX_PROP_LENGTH);
    if (NULL == p->playback_status) { return; }
    p->metadata = (mpris_metadata*) calloc(1, sizeof(mpris_metadata));
    mpris_metadata_init(p->metadata);
    printf("player: %p\n", p->player_name);
    printf("loop: %p\n", p->loop_status);
    printf("playback: %p\n", p->playback_status);
    printf("metadata: %p\n", p->metadata);
}

void mpris_metadata_unref(mpris_metadata *m)
{
    printf("album artist: %p\n", m->album_artist);
    printf("composer: %p\n", m->composer);
    printf("genre: %p\n", m->genre);
    printf("artist: %p\n", m->artist);
    printf("comment: %p\n", m->comment);
    printf("album: %p\n", m->album);
    printf("title: %p\n", m->title);
    free(m->album_artist);
    free(m->composer);
    free(m->genre);
    free(m->artist);
    free(m->comment);
    free(m->album);
    free(m->title);
    free(m);
}

void mpris_properties_unref(mpris_properties *p)
{
    mpris_metadata_unref(p->metadata);
    printf("player: %p\n", p->player_name);
    printf("loop: %p\n", p->loop_status);
    printf("playback: %p\n", p->playback_status);
    printf("metadata: %p\n", p->metadata);
    free(p->player_name);
    free(p->loop_status);
    free(p->playback_status);
    free(p);
}

DBusMessage* call_dbus_method(DBusConnection* conn, char* destination, char* path, char* interface, char* method)
{
    if (NULL == conn) { return NULL; }

    DBusMessage* msg;
    DBusPendingCall* pending;

    // create a new method call and check for errors
    msg = dbus_message_new_method_call(destination, path, interface, method);
    if (NULL == msg) { return NULL; }

    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, DBUS_CONNECTION_TIMEOUT)) {
        //fprintf(stderr, "Out Of Memory!\n");
        dbus_message_unref(msg);
        return NULL;
    }
    if (NULL == pending) {
        //fprintf(stderr, "Pending Call Null\n");
        dbus_message_unref(msg);
        return NULL;
    }
    dbus_connection_flush(conn);

    // free message
    dbus_message_unref(msg);

    // block until we receive a reply
    dbus_pending_call_block(pending);

    DBusMessage* reply;
    // get the reply message
    reply = dbus_pending_call_steal_reply(pending);
    if (NULL == reply) {
        //fprintf(stderr, "Reply Null\n");
    }

    // free the pending message handle
    dbus_pending_call_unref(pending);
    dbus_message_unref(reply);

    return reply;
}

double extract_double_var(DBusMessageIter *iter, DBusError *error)
{
    double result = 0;

    if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(iter)) {
        dbus_set_error_const(error, "iter_should_be_variant", "This message iterator must be have variant type");
        return 0;
    }

    DBusMessageIter variantIter;
    dbus_message_iter_recurse(iter, &variantIter);
    if (DBUS_TYPE_DOUBLE == dbus_message_iter_get_arg_type(&variantIter)) {
        dbus_message_iter_get_basic(&variantIter, &result);
        return result;
    }
    return 0;
}

void extract_string_var(DBusMessageIter *iter, char** value, DBusError *error)
{
    if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(iter)) {
        dbus_set_error_const(error, "iter_should_be_variant", "This message iterator must be have variant type");
        return;
    }

    DBusMessageIter variantIter;
    dbus_message_iter_recurse(iter, &variantIter);
    if (DBUS_TYPE_OBJECT_PATH == dbus_message_iter_get_arg_type(&variantIter)) {
        dbus_message_iter_get_basic(&variantIter, value);
    }
    if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&variantIter)) {
        dbus_message_iter_get_basic(&variantIter, value);
    }
    if (DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&variantIter)) {
        DBusMessageIter arrayIter;
        dbus_message_iter_recurse(&variantIter, &arrayIter);
        while (true) {
            // todo(marius): load all elements of the array
            if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&arrayIter)) {
                dbus_message_iter_get_basic(&arrayIter, value);
            }
            if (!dbus_message_iter_has_next(&arrayIter)) { break; }
            dbus_message_iter_next(&arrayIter);
        }
    }
    return;
}

int32_t extract_int32_var(DBusMessageIter *iter, DBusError *error)
{
    int32_t result = 0;
    if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(iter)) {
        dbus_set_error_const(error, "iter_should_be_variant", "This message iterator must be have variant type");
        return 0;
    }

    DBusMessageIter variantIter;
    dbus_message_iter_recurse(iter, &variantIter);

    if (DBUS_TYPE_INT32 == dbus_message_iter_get_arg_type(&variantIter)) {
        dbus_message_iter_get_basic(&variantIter, &result);
        return result;
    }
    return 0;
}

int64_t extract_int64_var(DBusMessageIter *iter, DBusError *error)
{
    int64_t result = 0;
    if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(iter)) {
        dbus_set_error_const(error, "iter_should_be_variant", "This message iterator must be have variant type");
        return 0;
    }

    DBusMessageIter variantIter;
    dbus_message_iter_recurse(iter, &variantIter);

    if (DBUS_TYPE_INT64 == dbus_message_iter_get_arg_type(&variantIter)) {
        dbus_message_iter_get_basic(&variantIter, &result);
        return result;
    }
    return 0;
}

bool extract_boolean_var(DBusMessageIter *iter,  DBusError *error)
{
    bool *result = false;

    if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(iter)) {
        dbus_set_error_const(error, "iter_should_be_variant", "This message iterator must be have variant type");
        return false;
    }

    DBusMessageIter variantIter;
    dbus_message_iter_recurse(iter, &variantIter);

    if (DBUS_TYPE_BOOLEAN == dbus_message_iter_get_arg_type(&variantIter)) {
        dbus_message_iter_get_basic(&variantIter, &result);
        return result;
    }
    return false;
}

void load_metadata(DBusMessageIter *iter, mpris_metadata *track, DBusError *error)
{
    if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(iter)) {
        dbus_set_error_const(error, "iter_should_be_variant", "This message iterator must be have variant type");
        return;
    }

    DBusMessageIter variantIter;
    dbus_message_iter_recurse(iter, &variantIter);
    if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&variantIter)) {
        dbus_set_error_const(error, "variant_should_be_array", "This variant reply message must have array content");
        return;
    }
    DBusMessageIter arrayIter;
    dbus_message_iter_recurse(&variantIter, &arrayIter);
    while (true) {
        char* key;
        if (DBUS_TYPE_DICT_ENTRY == dbus_message_iter_get_arg_type(&arrayIter)) {
            DBusMessageIter dictIter;
            dbus_message_iter_recurse(&arrayIter, &dictIter);
            if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&dictIter)) {
                dbus_set_error_const(error, "missing_key", "This message iterator doesn't have key");
            }
            dbus_message_iter_get_basic(&dictIter, &key);

            if (!dbus_message_iter_has_next(&dictIter)) {
                continue;
            }
            dbus_message_iter_next(&dictIter);

            if (!strncmp(key, MPRIS_METADATA_BITRATE, strlen(MPRIS_METADATA_BITRATE))) {
                track->bitrate = extract_int32_var(&dictIter, error);
            }
            if (!strncmp(key, MPRIS_METADATA_ART_URL, strlen(MPRIS_METADATA_ART_URL))) {
                extract_string_var(&dictIter, &track->art_url, error);
            }
            if (!strncmp(key, MPRIS_METADATA_LENGTH, strlen(MPRIS_METADATA_LENGTH))) {
                track->length = extract_int64_var(&dictIter, error);
            }
            if (!strncmp(key, MPRIS_METADATA_TRACKID, strlen(MPRIS_METADATA_TRACKID))) {
                extract_string_var(&dictIter, &track->track_id, error);
            }
            if (!strncmp(key, MPRIS_METADATA_ALBUM_ARTIST, strlen(MPRIS_METADATA_ALBUM_ARTIST))) {
                 extract_string_var(&dictIter, &track->album_artist,error);
            } else if (!strncmp(key, MPRIS_METADATA_ALBUM, strlen(MPRIS_METADATA_ALBUM))) {
                 extract_string_var(&dictIter, &track->album, error);
            }
            if (!strncmp(key, MPRIS_METADATA_ARTIST, strlen(MPRIS_METADATA_ARTIST))) {
                extract_string_var(&dictIter, &track->artist, error);
            }
            if (!strncmp(key, MPRIS_METADATA_COMMENT, strlen(MPRIS_METADATA_COMMENT))) {
                extract_string_var(&dictIter, &track->comment, error);
            }
            if (!strncmp(key, MPRIS_METADATA_TITLE, strlen(MPRIS_METADATA_TITLE))) {
                extract_string_var(&dictIter, &track->title, error);
            }
            if (!strncmp(key, MPRIS_METADATA_TRACK_NUMBER, strlen(MPRIS_METADATA_TRACK_NUMBER))) {
                track->track_number = extract_int32_var(&dictIter, error);
            }
            if (!strncmp(key, MPRIS_METADATA_URL, strlen(MPRIS_METADATA_URL))) {
                 extract_string_var(&dictIter, &track->url, error);
            }
            if (dbus_error_is_set(error)) {
                //fprintf(stderr, "error: %s, %s\n", key, error->message);
                dbus_error_free(error);
            }
        }
        if (!dbus_message_iter_has_next(&arrayIter)) {
            break;
        }
        dbus_message_iter_next(&arrayIter);
    }
    return;
}

void get_player_identity(DBusConnection *conn, const char* destination, char** player_name)
{
    if (NULL == conn) { return; }
    if (NULL == destination) { return; }
    if (NULL == player_name) { return; }

    DBusMessage* msg;
    DBusPendingCall* pending;
    DBusMessageIter params;

    char* interface = DBUS_PROPERTIES_INTERFACE;
    char* method = DBUS_METHOD_GET;
    char* path = MPRIS_PLAYER_PATH;
    char* arg_interface = MPRIS_PLAYER_NAMESPACE;
    char* arg_identity = MPRIS_ARG_PLAYER_IDENTITY;

    // create a new method call and check for errors
    msg = dbus_message_new_method_call(destination, path, interface, method);
    if (NULL == msg) { return; }

    // append interface we want to get the property from
    dbus_message_iter_init_append(msg, &params);
    if (!dbus_message_iter_append_basic(&params, DBUS_TYPE_STRING, &arg_interface)) {
        goto _unref_message_error;
    }

    dbus_message_iter_init_append(msg, &params);
    if (!dbus_message_iter_append_basic(&params, DBUS_TYPE_STRING, &arg_identity)) {
        goto _unref_message_error;
    }

    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, DBUS_CONNECTION_TIMEOUT)) {
        goto _unref_message_error;
    }
    if (NULL == pending) { goto _unref_pending_error; }
    dbus_connection_flush(conn);


    // block until we receive a reply
    dbus_pending_call_block(pending);

    DBusMessage* reply;
    // get the reply message
    reply = dbus_pending_call_steal_reply(pending);
    if (NULL == reply) { goto _unref_reply_error; }

    DBusMessageIter rootIter;
    if (dbus_message_iter_init(reply, &rootIter)) {
        extract_string_var(&rootIter, player_name, NULL);
    }
    dbus_message_unref(msg);
    dbus_pending_call_unref(pending);
    dbus_message_unref(reply);
    return;

_unref_reply_error:
    {
        dbus_message_unref(reply);
        goto _unref_pending_error;
    }
_unref_pending_error:
    {
        // free the pending message handle
        dbus_pending_call_unref(pending);
        goto _unref_message_error;
    }
_unref_message_error:
    {
        // free message
        dbus_message_unref(msg);
        goto _exit_error;
    }
_exit_error:
    {
        return;
    }
}

void get_mpris_properties(DBusConnection* conn, char* destination, mpris_properties* properties)
{
    if (NULL == conn) { return; }
    if (NULL == properties) { return; }
    if (NULL == destination) { return; }

    get_player_identity(conn, destination, &properties->player_name);

    DBusMessage* msg;
    DBusPendingCall* pending;
    DBusMessageIter params;

    char* interface = DBUS_PROPERTIES_INTERFACE;
    char* method = DBUS_METHOD_GET_ALL;
    char* path = MPRIS_PLAYER_PATH;
    char* arg_interface = MPRIS_PLAYER_INTERFACE;

    // create a new method call and check for errors
    msg = dbus_message_new_method_call(destination, path, interface, method);
    if (NULL == msg) { return; }

    // append interface we want to get the property from
    dbus_message_iter_init_append(msg, &params);
    if (!dbus_message_iter_append_basic(&params, DBUS_TYPE_STRING, &arg_interface)) {
        //fprintf(stderr, "Out Of Memory!\n");
    }

    // send message and get a handle for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, DBUS_CONNECTION_TIMEOUT)) {
        //fprintf(stderr, "Out Of Memory!\n");
    }
    if (NULL == pending) {
        //fprintf(stderr, "Pending Call Null\n");
    }
    dbus_connection_flush(conn);

    // free message
    dbus_message_unref(msg);

    // block until we receive a reply
    dbus_pending_call_block(pending);

    DBusMessage* reply;
    // get the reply message
    reply = dbus_pending_call_steal_reply(pending);
    if (NULL == reply) {
        //fprintf(stderr, "Reply Null\n");
    }

    // free the pending message handle
    dbus_pending_call_unref(pending);

    DBusMessageIter rootIter;
    if (dbus_message_iter_init(reply, &rootIter) && DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&rootIter)) {
        DBusMessageIter arrayElementIter;
        dbus_message_unref(reply);

        dbus_message_iter_recurse(&rootIter, &arrayElementIter);
        while (true) {
            char* key;
            if (DBUS_TYPE_DICT_ENTRY == dbus_message_iter_get_arg_type(&arrayElementIter)) {
                DBusError err = {};
                DBusMessageIter dictIter;
                dbus_message_iter_recurse(&arrayElementIter, &dictIter);
                if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&dictIter)) {
                    dbus_set_error_const(&err, "missing_key", "This message iterator doesn't have key");
                }
                dbus_message_iter_get_basic(&dictIter, &key);

                if (!dbus_message_iter_has_next(&dictIter)) {
                    continue;
                }
                dbus_message_iter_next(&dictIter);

                if (!strncmp(key, MPRIS_PNAME_CANCONTROL, strlen(MPRIS_PNAME_CANCONTROL))) {
                     properties->can_control = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_CANGONEXT, strlen(MPRIS_PNAME_CANGONEXT))) {
                     properties->can_go_next = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_CANGOPREVIOUS, strlen(MPRIS_PNAME_CANGOPREVIOUS))) {
                   properties->can_go_previous = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_CANPAUSE, strlen(MPRIS_PNAME_CANPAUSE))) {
                    properties->can_pause = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_CANPLAY, strlen(MPRIS_PNAME_CANPLAY))) {
                    properties->can_play = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_CANSEEK, strlen(MPRIS_PNAME_CANSEEK))) {
                    properties->can_seek = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_LOOPSTATUS, strlen(MPRIS_PNAME_LOOPSTATUS))) {
                    extract_string_var(&dictIter, &properties->loop_status, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_METADATA, strlen(MPRIS_PNAME_METADATA))) {
                    load_metadata(&dictIter, properties->metadata, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_PLAYBACKSTATUS, strlen(MPRIS_PNAME_PLAYBACKSTATUS))) {
                     extract_string_var(&dictIter, &properties->playback_status, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_POSITION, strlen(MPRIS_PNAME_POSITION))) {
                      properties->position= extract_int64_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_SHUFFLE, strlen(MPRIS_PNAME_SHUFFLE))) {
                    properties->shuffle = extract_boolean_var(&dictIter, &err);
                }
                if (!strncmp(key, MPRIS_PNAME_VOLUME, strlen(MPRIS_PNAME_VOLUME))) {
                     properties->volume = extract_double_var(&dictIter, &err);
                }
                if (dbus_error_is_set(&err)) {
                    //fprintf(stderr, "error: %s\n", err.message);
                }
            }
            if (!dbus_message_iter_has_next(&arrayElementIter)) {
                break;
            }
            dbus_message_iter_next(&arrayElementIter);
        }
    }
}

char* get_player_namespace(DBusConnection* conn)
{
    if (NULL == conn) { return NULL; }

    char* player_namespace = NULL;

    char* method = DBUS_METHOD_LIST_NAMES;
    char* destination = DBUS_DESTINATION;
    char* path = DBUS_PATH;
    char* interface = DBUS_INTERFACE;
    const char* mpris_namespace = MPRIS_PLAYER_NAMESPACE;
    DBusMessage* reply = call_dbus_method(conn, destination, path, interface, method);
    if (NULL == reply) { return NULL; }

    DBusMessageIter rootIter;
    if (dbus_message_iter_init(reply, &rootIter) &&
        DBUS_TYPE_ARRAY == dbus_message_iter_get_arg_type(&rootIter)) {
        DBusMessageIter arrayElementIter;

        dbus_message_iter_recurse(&rootIter, &arrayElementIter);
        while (true) {
            if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&arrayElementIter)) {
                char* str;
                dbus_message_iter_get_basic(&arrayElementIter, &str);
                if (!strncmp(str, mpris_namespace, strlen(mpris_namespace))) {
                    player_namespace = str;
                    break;
                }
            }
            if (!dbus_message_iter_has_next(&arrayElementIter)) {
                break;
            }
            dbus_message_iter_next(&arrayElementIter);
        }
    }

    return player_namespace;
}

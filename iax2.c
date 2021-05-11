/*
 * $Id: iax2.c 579 2006-05-08 14:45:03Z arnaldo $
 */

#include "priax.h"

/* max lenght of dial string */
#define DIALSTRING_LEN 256

/* prepare dial string */
static void dialstring(char *dest, int dest_len, char *number)
{
    IAX2ptr p = IAX2Get();

    char *str = p->number();
    int   len = sizeof(p->private.current->number);

    /* copy real number into private array of calls */
    strncpy(str, number, len);

    /* copy dial string to dest */
    memset(dest, 0, dest_len);
    snprintf(dest, dest_len, "%s:%s@%s/%s",
            p->private.reginfo.user, p->private.reginfo.pass,
            p->private.reginfo.host, number);
}

/* load ring file */
static void ringload(IAX2_CallDef *call, char *filename)
{
    struct stat st;
    FILE *fp;

    /* if there is a problem loading sound, we use this default */
    void default_ring() {
        char *tone = "\a\a\a";
        int   len = strlen(tone);

        call->sound.len      = len;
        call->sound.data     = (short *) calloc(len, sizeof(short));
        call->sound.malloced = 1;
        call->sound.repeat   = 768;

        memcpy((char *) call->sound.data, tone, len);
    }

    if(filename == NULL) {
        default_ring();
        return;
    }

    if(stat(filename, &st) < 0) {
        debug(_("Couldn't stat %s: %s\n"),
                filename, strerror(errno));
        return;
    }

    if(!(fp = fopen(filename, "r"))) {
        debug(_("Couldn't open %s: %s\n"),
                filename, strerror(errno));
        default_ring();
        return;
    }

    /* save sound lenght, state and content into iaxclient's sound */
    call->sound.len      = st.st_size;
    call->sound.data     = (short *) calloc(st.st_size, sizeof(short));
    call->sound.malloced = 1;
    call->sound.repeat   = -1;

    fread(call->sound.data, st.st_size, 1, fp);
    fclose(fp);
}

/* load ringback file */
static void ringbackload(struct iaxc_sound *sound, char *filename)
{
    struct stat st;
    FILE *fp;

    if(stat(filename, &st) < 0) {
        debug(_("Couldn't stat %s: %s\n"),
                filename, strerror(errno));
        return;
    }

    if(!(fp = fopen(filename, "r"))) {
        debug(_("Couldn't open %s: %s\n"),
                filename, strerror(errno));
        return;
    }

    /* save sound lenght, state and content into iaxclient's sound */
    sound->len      = st.st_size;
    sound->data     = (short *) calloc(st.st_size, sizeof(short));
    sound->malloced = 1;
    sound->repeat   = -1;

    fread(sound->data, st.st_size, 1, fp);
    fclose(fp);
}

static int ringspeaker(void *call_id)
{
    int times = 2;

    for(; times; times--) {
        int i;

        for(i = 0; i < 40; i++) {
            fprintf(stdout, "\a");
            for(i = 0; i < 5000; i++);
            fflush(stdout);
        }
        for(i = 0; i < 100000; i++);
    }

    return EXIT_SUCCESS;
}

/* start ringing */
static void ringstart(int id)
{
    IAX2ptr              p = IAX2Get();
#ifndef WIN32
    IAX2_PrivateDef    *pp = &p->private;
#endif
    IAX2_CallDef     *call = &p->private.callinfo[id];
    IAX2_CallDef *callwait = &p->private.callwaiting;

    ShowMainWindow();
    PRIAX_SET_CURRENT_STATE(PRIAX_STATE_RINGING);
    PRIAX_SET_CURRENT_STATE(PRIAX_STATE_INCOMING);

#ifndef WIN32
    /* check if sound devices are writable and not opened */
    if(is_opened(pp->snd_devices[pp->snd_input].name)  ||
       is_opened(pp->snd_devices[pp->snd_output].name) ||
       is_opened(pp->snd_devices[pp->snd_ring].name)       ) {
        PRIAX_LCDText(LCD_Register, _("ERRO: Dispositivo de audio em uso !"));
        return;
    }
#endif

    /* play using iaxclient's RING DEVICE (1 below) */
    if(PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_ACTIVE))
        iaxc_play_sound(&callwait->sound, 1);
    else {
        iaxc_play_sound(&call->sound, 1);
	    call->speaker_thread = SDL_CreateThread(ringspeaker, (void *) id);
    }
}

/* start ringing */
static void ringbackstart()
{
    IAX2ptr p = IAX2Get();

    iaxc_play_sound(&p->private.rb_sound, 0);

    PRIAX_SET_CURRENT_STATE(PRIAX_STATE_RINGING);
    PRIAX_SET_CURRENT_STATE(PRIAX_STATE_OUTGOING);
}

/* stop ringback */
static void ringbackstop() { iaxc_stop_sound(IAX2Get()->private.index); }

/* stop ringing */
static void ringstop(int id)
{
    IAX2ptr p = IAX2Get();

    if(id < 0) /* -1 means current call */
        id = p->private.index;

    iaxc_stop_sound(id);
    PRIAX_UNSET_CURRENT_STATE(PRIAX_STATE_RINGING);
}

/* priv_select_call */
static int priv_select_call(int index)
{
    IAX2ptr p = IAX2Get();
    int in_use = 0;

    if(PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_ACTIVE))
        p->hold();

    p->private.index = index;
    p->private.current = &p->private.callinfo[index];

    if(PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_WAITING)) {
        PRIAX_LCDClear(LCD_TextInfo);
        PRIAX_AnimSet(RecvAnim, p->number());
        PRIAX_AnimControl(AnimPlay);
    }

    if(PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_ACTIVE)) {
        PRIAX_AnimSet(TalkAnim, p->number());
        PRIAX_AnimControl(AnimPlay);
        iaxc_select_call(index);
        p->unhold();
        in_use = 1;

    } else {
        iaxc_select_call(-1);
        PRIAX_AnimControl(AnimStop);
    }

    PRIAX_LCDText(LCD_Register, _("Line %d"), index+1);
    return in_use;
}

/* prev_call */
static int priv_prev_call()
{
    IAX2ptr p = IAX2Get();
    if(p->private.index)
        p->private.index--;
    return priv_select_call(p->private.index);
}

/* next_call */
static int priv_next_call()
{
    IAX2ptr p = IAX2Get();
    if(p->private.index != (MAXLINES-1))
        p->private.index++;
    return priv_select_call(p->private.index);
}

/* iax event: url */
static int ev_url(struct iaxc_ev_url url)
{
    switch(url.type) {
        case IAXC_URL_URL:
        case IAXC_URL_LDCOMPLETE:
        case IAXC_URL_LINKURL:
        case IAXC_URL_LINKREJECT:
        case IAXC_URL_UNLINK:
            break;
    }

    return 0;
}

/* iax event: text */
static int ev_text(struct iaxc_ev_text text)
{
    IAX2ptr p = IAX2Get();

    /* handle status messages */
    void handle_status()
    {
        char tmp[30];
        static int line_id = -1;

        if(strncmp(text.message, "Call disconnected by remote", 28) == 0) {
            if(PRIAX_CHECK_STATE(line_id, PRIAX_STATE_WAITING))
                PRIAX_LCDClear(LCD_TextInfo);
            if(line_id != p->private.last_active)
                p->hangup(p->private.last_active);
            else
                p->hangup(line_id);

        } else if(strstr(text.message, "Hanging")) {
            sscanf(text.message, "Hanging up call %d", &line_id);
            p->hangup(line_id);

        } else if(strncmp(text.message, "Incoming call on line", 21) == 0) {
            sscanf(text.message, "Incoming call on line %d", &line_id);
            p->private.last_active = line_id;

            if(PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_ACTIVE)) {
                snprintf(tmp, sizeof(tmp), "%s waiting on line %d...", 
                        p->private.callinfo[line_id].number, line_id+1);
                PRIAX_LCDText(LCD_TextInfo, tmp);
                ringstart(line_id);
                PRIAX_SET_STATE(line_id, PRIAX_STATE_RINGING);
                PRIAX_SET_STATE(line_id, PRIAX_STATE_INCOMING);
                PRIAX_SET_STATE(line_id, PRIAX_STATE_WAITING);
            } else {

                /* come back from callreg */
                CR_LCDClear();
                PRIAX_Event();

                PRIAX_AnimSet(RecvAnim, p->number());
                PRIAX_AnimControl(AnimPlay);
                ringstart(p->private.index);
            }

            /* as long as the array's calls placement is controlled by
             * iaxclient, we might want to move the current call to the
             * proper position, to align with the library.
             *
             * "Call n accepted" is the first string we receive after a dial
             * command, so we must move the call here.
             *
             * FIXME: this is most likely a call to priv_select_call(), we shouldn't
             * have this code here.
             */
        } else if(strstr(text.message, "accepted")) {
            sscanf(text.message, "Call %d accepted", &line_id);
            if(p->private.index != line_id) {
                memmove(&p->private.callinfo[line_id], p->private.current, 
                        sizeof(IAX2_CallDef) - sizeof(struct iaxc_sound));
                memset(&p->private.current, 0, sizeof(IAX2_CallDef) - sizeof(struct iaxc_sound));
                p->private.index = line_id;
                p->private.current = &p->private.callinfo[line_id];
                iaxc_select_call(line_id);

                if(PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_WAITING)) {
                    PRIAX_AnimSet(RecvAnim, p->number());
                } else
                    PRIAX_AnimSet(TalkAnim, p->number());
                PRIAX_AnimControl(AnimPlay);
                PRIAX_LCDText(LCD_Register, _("Line %d"), line_id+1);
            }

            if(PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_OUTGOING))
                ringbackstart();

        } else if(strstr(text.message, "answered")) {
            /* start animation */
            PRIAX_AnimSet(TalkAnim, p->number());
            PRIAX_AnimControl(AnimPlay);

            /* mark line as in use */
            PRIAX_SET_CURRENT_STATE(PRIAX_STATE_ACTIVE);

            ringbackstop();

            /* unmute mic */
            p->unmute();
        } else if(strstr(text.message, "rejected")) {
            /* stop animation */
            PRIAX_AnimControl(AnimStop);
            if(line_id == -1)
                p->hangup(p->private.index);

        } else if(strstr(text.message, "ringing")) {
            debug("ringing ?\n");
        } else
            debug(_("IAX unhandled text ev: %s\n"), text.message);
    }

    switch(text.type) {
        case IAXC_TEXT_TYPE_STATUS:
            handle_status();
            break;
        case IAXC_TEXT_TYPE_NOTICE:
        case IAXC_TEXT_TYPE_ERROR:
        case IAXC_TEXT_TYPE_FATALERROR:
        case IAXC_TEXT_TYPE_IAX:
            PRIAX_LCDText(LCD_Register, "%s: %s", 
                    p->private.current->number, text.message);
            break;
    }

    return 0;
}

/* iax event: video */
static int ev_video(struct iaxc_ev_video video) { return 0; }

/* iax event: state */
static int ev_state(struct iaxc_ev_call_state call)
{
    IAX2ptr p = IAX2Get();
    IAX2_CallDef *this;
    char *str;
    int   len = sizeof(p->private.current->number);

    if(call.callNo > MAXLINES) {
        debug(_("WARNING: reached max iax lines, ignoring event\n"));
        return 0;
    }

    /* point to call where the event occurred */
    this = &p->private.callinfo[call.callNo];
    str = this->number;

    /* save remote number */
    if(call.remote) {
        memset(str, 0, len);
        strncpy(str, call.remote, len);
    }

    return 0;
}

/* iax event: levels */
static int ev_levels(struct iaxc_ev_levels levels) { return 0; }

/* iax event: netstat */
static int ev_netstat(struct iaxc_ev_netstats netstats) { return 0; }

/* iax event: registration */
static int ev_registration(struct iaxc_ev_registration reg)
{
    IAX2ptr p = IAX2Get();

    switch(reg.reply) {
        case IAXC_REGISTRATION_REPLY_ACK:
            PRIAX_LCDText(LCD_Register, _("Online [%s at %s]"),
                    p->private.reginfo.user, p->private.reginfo.host);

            /* if there is voicemail, blink! */
            debug("Mailbox msgcount: %d\n", reg.msgcount);
            if(reg.msgcount)
                PRIAX_VoiceMailON();
            else
                PRIAX_VoiceMailOFF();

            /* save registered state */
            p->private.reginfo.state = 1;

            break;
        case IAXC_REGISTRATION_REPLY_REJ:
            PRIAX_LCDText(LCD_Register, _("Registration rejected"));
            break;
        case IAXC_REGISTRATION_REPLY_TIMEOUT:
            PRIAX_LCDText(LCD_Register, _("Registration timed-out"));
            break;
    }

    return 0;
}

/* iaxclient's callback */
static int callback(iaxc_event in)
{
    switch(in.type) {
        case IAXC_EVENT_URL:
            return ev_url(in.ev.url);
        case IAXC_EVENT_TEXT:
            return ev_text(in.ev.text);
        case IAXC_EVENT_VIDEO:
            return ev_video(in.ev.video);
        case IAXC_EVENT_STATE:
            return ev_state(in.ev.call);
        case IAXC_EVENT_LEVELS:
            return ev_levels(in.ev.levels);
        case IAXC_EVENT_NETSTAT:
            return ev_netstat(in.ev.netstats);
        case IAXC_EVENT_REGISTRATION:
            return ev_registration(in.ev.reg);
        default:
            debug("IAX event %d unhandled yet.\n", in.type);
    }

    return 0;
}

/* register */
void priv_setregistry(char *user, char *pass, char *host)
{
    IAX2_RegistryDef reg;
    IAX2ptr p = IAX2Get();

    if(!user || !pass || !host) return;

    memset(&reg, 0, sizeof(reg));
    strncpy(reg.user, user, sizeof(reg.user));
    strncpy(reg.pass, pass, sizeof(reg.pass));
    strncpy(reg.host, host, sizeof(reg.host));

    PRIAX_LCDText(LCD_Register, _("Connecting %s@%s"), user, host);

    /* register at iaxclient */
    iaxc_register(user, pass, host);

    /* copy reg into private */
    memset(&p->private.reginfo,    0, sizeof(p->private.reginfo));
    memcpy(&p->private.reginfo, &reg, sizeof(p->private.reginfo));
}

/* set caller id */
void priv_setcallerid(char *name, char *number)
{ 
    if(!name || !number) return;
    iaxc_set_callerid(name, number);
}

/* dial */
static int priv_dial(char *number)
{
    IAX2ptr p = IAX2Get();
    char temp[DIALSTRING_LEN];
    static char numbah[64];

    if(!strlen(number))
        return -1;

    /* keep a copy of last-dialed number - sometimes numeric_buffer
     * is cleaned before we save it. */
    strncpy(numbah, number, sizeof(numbah));

    if(!p->private.reginfo.state) /* not registered */
        return -1;
    else
        /* prepare dial string and copy it into current call */
        dialstring(temp, sizeof(temp), numbah);

    /* log into 'calls dialled' */
    PRIAX_LogAdd(LogSend, numbah);

    /* clear register screen - we don't know if a hangup occurred
     * just before the dial event, so we must clean LCD_Register
     */
    PRIAX_LCDClear(LCD_Register);

    /* call */
    iaxc_call(temp);

    PRIAX_AnimSet(DialAnim, numbah);
    PRIAX_AnimControl(AnimPlay);

    PRIAX_SET_CURRENT_STATE(PRIAX_STATE_OUTGOING);

    /* flush event's buffer */
    zero_numeric_buffer();
    return 0;
}

/* hangup */
static void priv_hangup(int index)
{
    IAX2ptr p = IAX2Get();
    char *str = p->number();
    int   len = sizeof(p->private.current->number);

    iaxc_reject_call_number(index);

    PRIAX_DeactiveLed(HoldLed);
    PRIAX_DeactiveLed(TransferLed);

    /* match iaxclient's call number against PRIAX's one */
    if(index == p->private.index) {
        /* stop ring sound */
        ringstop(p->private.index);

        /* update LCD */
        if(strlen(p->private.current->number))
            PRIAX_LCDText(LCD_Register, _("Hangup [%s] on line [%d]."), 
                    p->private.current->number, index+1);
        else {
            if(p->private.state)
                PRIAX_LCDText(LCD_Register, _("Ready."), p->private.current->number);
            else
                PRIAX_LCDText(LCD_Register, _("Not registered."), p->private.current->number);
        }

        /* log this call */
        if(p->private.current->answered)
            PRIAX_LogAdd(LogRecv, str);
        else if(PRIAX_CHECK_STATE(index, PRIAX_STATE_INCOMING))
            PRIAX_LogAdd(LogLost, str);

        /* clear number of this call (current) */
        memset(str, 0, len);

        /* stop animation anyway */
        PRIAX_AnimControl(AnimStop);

        iaxc_select_call(-1);
    }

    PRIAX_FREE_STATE(index);
}

/* pickup - answer call */
static void priv_pickup()
{
    IAX2ptr p = IAX2Get();

    /* stop ringing */
    ringstop(p->private.index);

    /* mark as answered */
    p->private.current->answered = 1;

    /* pickup */
    iaxc_set_audio_output(0);
    iaxc_select_call(p->private.index);
    iaxc_answer_call(p->private.index);

    PRIAX_AnimSet(TalkAnim, p->number());
    PRIAX_AnimControl(AnimPlay);

    PRIAX_SET_CURRENT_STATE(PRIAX_STATE_ACTIVE);
}

/* hold */
static void priv_hold()
{
    IAX2ptr p = IAX2Get();

    iaxc_quelch(p->private.index, 1);
    iaxc_select_call(-1);
}

/* unhold */
static void priv_unhold()
{
    IAX2ptr p = IAX2Get();
    int id = p->private.index;

    iaxc_unquelch(id);
    iaxc_select_call(id);
}

/* mute */
static void priv_mute()
{
    IAX2ptr p = IAX2Get();

    p->private.current->mute_state = 1;
    iaxc_set_silence_threshold(0);
}

/* unmute */
static void priv_unmute()
{
    IAX2ptr p = IAX2Get();

    p->private.current->mute_state = 0;
    iaxc_set_silence_threshold(1);
}

/* dtmf */
static void priv_dtmf(char digit)
{
    IAX2ptr p = IAX2Get();

    if(p->private.reginfo.state) {
        PRIAX_LCDText(LCD_Register, _("Sending DTMF: %c"), digit);
        iaxc_send_dtmf(digit);
    } else
        PRIAX_LCDText(LCD_Register, _("Not registered"));
}

/* transfer */
static void priv_transfer()
{
    IAX2ptr p = IAX2Get();
    char *digit;

    for(digit = p->conf->atxferexten; *digit; digit++)
        p->dtmf(*digit);
}

/* related to current call: boolean is_active */
static int priv_is_active() { return PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_ACTIVE); }

/* related to current call: integer selected_call, -1 if none */
static int priv_selected() { return iaxc_selected_call(); }

/* related to current call: boolean is_ringing */
static int priv_is_ringing() { return PRIAX_CHECK_CURRENT_STATE(PRIAX_STATE_RINGING); }

/* related to current call: ptr number */
static char *priv_number()
{
    IAX2ptr p = IAX2Get();

    return p->private.current->number;
}

/* finish iax2 */
static void priv_quit()
{
    iaxc_shutdown();
}

/* set sound devices */
static int priv_setdevices(char *in, char *out, char *ring)
{
    IAX2ptr                  p        =  IAX2Get();
    IAX2_PrivateDef          *pp      = &p->private;
    struct iaxc_audio_device *current =  NULL;
    int i;

    /* XXX */
    memset(&pp->snd_devices, 0, sizeof(struct iaxc_audio_device));

    /* get iaxclient configured sound devices */
    iaxc_audio_devices_get(&pp->snd_devices, &pp->snd_ndevs, &pp->snd_input, 
            &pp->snd_output, &pp->snd_ring);
    debug(_("Number of sound devices found: %d\n"), pp->snd_ndevs);
    debug(_("Input currently set: %s\n"), pp->snd_devices[pp->snd_input].name);
    debug(_("Output currently set: %s\n"), pp->snd_devices[pp->snd_output].name);
    debug(_("Ring currently set: %s\n"), pp->snd_devices[pp->snd_ring].name);

    for(i = 0, current = pp->snd_devices; i < pp->snd_ndevs; i++, current++) {
        if(strncmp(current->name, in, strlen(current->name)) == 0)
            pp->snd_input = i;
        if(strncmp(current->name, out, strlen(current->name)) == 0)
            pp->snd_output = i;
        if(strncmp(current->name, ring, strlen(current->name)) == 0)
            pp->snd_ring = i;
    }

    debug(_("Configured input : %s\n"), pp->snd_devices[pp->snd_input].name);
    debug(_("Configured output: %s\n"), pp->snd_devices[pp->snd_output].name);
    debug(_("Configured ring  : %s\n"), pp->snd_devices[pp->snd_ring].name);

    return iaxc_audio_devices_set(pp->snd_input, pp->snd_output, pp->snd_ring);
}

/* main data */
static struct _IAX2Data priv_data = {
    .setregistry  = priv_setregistry,
    .setcallerid  = priv_setcallerid,
    .dial         = priv_dial,
    .hangup       = priv_hangup,
    .pickup       = priv_pickup,
    .prev_call    = priv_prev_call,
    .next_call    = priv_next_call,
    .select_call  = priv_select_call,
    .hold         = priv_hold,
    .unhold       = priv_unhold,
    .mute         = priv_mute,
    .unmute       = priv_unmute,
    .dtmf         = priv_dtmf,
    .transfer     = priv_transfer,
    .is_active    = priv_is_active,
    .is_ringing   = priv_is_ringing,
    .selected     = priv_selected,
    .number       = priv_number,
    .setdevices   = priv_setdevices,
};

/* initialize iax2 */
static void iax2init()
{
    int i;
    /* iaxclient filter flags */
    int filter_flags = 
        IAXC_FILTER_AGC        |  /* automatic gain control */
        //IAXC_FILTER_AAGC     |  /* analog (mixer based) gain control */
        IAXC_FILTER_CN          /* Send CN frames when silence detected */
        //IAXC_FILTER_DENOISE     /* VAD - lesser bandwidth */
        //IAXC_FILTER_ECHO        /* echo cancellation - SEGFAULTS */
        ;

    /* iaxclient format flags */
    int format_flags = IAXC_FORMAT_GSM      | 
                       IAXC_FORMAT_SPEEX    | 
                       IAXC_FORMAT_ULAW     | 
                       IAXC_FORMAT_ALAW;
    IAX2ptr p = &priv_data;

    if(iaxc_initialize(AUDIO_INTERNAL_PA, MAXLINES)) {
        debug(_("Unable to init IAX2\n"));
        p->private.state = 0;
        p->private.current = &p->private.callinfo[p->private.index];

        return;
    } else {
        /* private data cleanup */
        memset(&p->private, 0, sizeof(p->private));

        /* set working state and ptr to current call */
        p->private.state = 1;
        p->private.current = &p->private.callinfo[p->private.index];
    }

    /* (preferred, allowed) */
    iaxc_set_formats(IAXC_FORMAT_GSM, format_flags);

    /* set filters */
    iaxc_set_filters(filter_flags);

    /* set iaxclient's callback manager */
    iaxc_set_event_callback(callback);

    /* start */
    iaxc_start_processing_thread();

    /* load ring tone into iaxclient's sound structure 
     * - last is for call waiting */
    for(i = 0; i <= MAXLINES; i++)
        ringload(&p->private.callinfo[i], PRIAX_Path_SOUNDS(IAX2_RING1));

    ringload(&p->private.callwaiting, NULL);
    ringbackload(&p->private.rb_sound, PRIAX_Path_SOUNDS(IAX2_RINGBACK));

    /* mute until asked to unmute - we don't want to receive
     * audio without our permission */
    iaxc_select_call(-1);

    /* prepare quit */
    my_atexit(priv_quit, "IAX2_Quit");
}

/* extern: return working ptr */
IAX2ptr IAX2Get()
{
    static int initialized = 0;

    if(!initialized) {
        iax2init();
        TOGGLE(initialized);
    }

    return &priv_data;
}

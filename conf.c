/*
 * $Id: conf.c 384 2005-12-16 19:22:35Z arnaldo $
 */

#include "priax.h"

PRIAX_Conf *PRIAX_GetConf(const char *filename)
{
    int i;
    FILE *fp;
    char temp[256], *p;
    static PRIAX_Conf conf;

    const static PRIAX_Conf default_conf = 
    { "1905", "1234", "127.0.0.1", "Foo Bar", "1905", "*100", "POSIX", "*8", "*2",
        "/dev/dsp",
        "/dev/dsp",
        "/dev/dsp",
    };

    struct {
        const char *key;
        char *buf;
        int   len;
    } Keys[] = {
        { "iax user",      conf.user,           sizeof(conf.user)           },
        { "iax pass",      conf.pass,           sizeof(conf.pass)           },
        { "iax host",      conf.host,           sizeof(conf.host)           },
        { "caller name",   conf.name,           sizeof(conf.name)           },
        { "caller number", conf.number,         sizeof(conf.number)         },
        { "mailbox",       conf.mailbox,        sizeof(conf.mailbox)        },
        { "pickupexten",   conf.pickupexten,    sizeof(conf.pickupexten)    },
        { "transferexten", conf.atxferexten,    sizeof(conf.atxferexten)    },
        { "language",      conf.lang,           sizeof(conf.lang)           },
        { "input device",  conf.snd_dev_input,  sizeof(conf.snd_dev_input)  },
        { "output device", conf.snd_dev_output, sizeof(conf.snd_dev_output) },
        { "ring device",   conf.snd_dev_ring,   sizeof(conf.snd_dev_ring)   },
    };

    static int KeysSize = sizeof(Keys)/sizeof(Keys[0]);

    memset(&conf, 0, sizeof(conf));
    memcpy(&conf, &default_conf, sizeof(conf));

    if((fp = fopen(filename, "r")) == NULL) 
        return (PRIAX_Conf *)&default_conf;

    while(!feof(fp)) {
        memset(temp, 0, sizeof(temp));
        if(fgets(temp, sizeof(temp), fp) == NULL) break;

        /* ignore blank lines or comments */
        if(ckline(temp)) continue;

        /* key match */
        for(i = 0; i < KeysSize; i++) {
            if(!strncmp(temp, Keys[i].key, strlen(Keys[i].key))) {
                p = temp;

                /* find '=' */
                while(*p != '=' && *p != '\0') p++;
                if(*p == '=') {
                    if(!strlen(++p)) break;
                }

                /* ignore space after '=' */
                while (*p == ' ' && *p != '\0') p++;
                if(*p == ' ') {
                    if(!strlen(++p)) break;
                }
                
                memset(Keys[i].buf, 0, Keys[i].len);
                strncpy(Keys[i].buf, fixstr(p), Keys[i].len);
            }
        }
    }

    fclose(fp);

    return &conf;
}

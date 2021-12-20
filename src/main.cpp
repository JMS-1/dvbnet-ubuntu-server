extern "C"
{
#include <libdvbv5/dvb-dev.h>
}

#include <stdio.h>

#define _(str) const_cast<char *>(str)

static int print_frontend_stats(
    struct dvb_v5_fe_parms *parms)
{
    char buf[512], *p;
    int rc, i, len, show, n_status_lines = 0;

    rc = dvb_fe_get_stats(parms);
    if (rc)
    {
        return -1;
    }

    p = buf;
    len = sizeof(buf);
    dvb_fe_snprintf_stat(parms, DTV_STATUS, NULL, 0, &p, &len, &show);

    for (i = 0; i < MAX_DTV_STATS; i++)
    {
        show = 1;

        dvb_fe_snprintf_stat(parms, DTV_QUALITY, _("Quality"),
                             i, &p, &len, &show);

        dvb_fe_snprintf_stat(parms, DTV_STAT_SIGNAL_STRENGTH, _("Signal"),
                             i, &p, &len, &show);

        dvb_fe_snprintf_stat(parms, DTV_STAT_CNR, _("C/N"),
                             i, &p, &len, &show);

        dvb_fe_snprintf_stat(parms, DTV_STAT_ERROR_BLOCK_COUNT, _("UCB"),
                             i, &p, &len, &show);

        dvb_fe_snprintf_stat(parms, DTV_BER, _("postBER"),
                             i, &p, &len, &show);

        dvb_fe_snprintf_stat(parms, DTV_PRE_BER, _("preBER"),
                             i, &p, &len, &show);

        dvb_fe_snprintf_stat(parms, DTV_PER, _("PER"),
                             i, &p, &len, &show);
        if (p != buf)
        {
            enum dvb_quality qual;
            int color;

            qual = dvb_fe_retrieve_quality(parms, 0);

            switch (qual)
            {
            case DVB_QUAL_POOR:
                color = 31;
                break;
            case DVB_QUAL_OK:
                color = 36;
                break;
            case DVB_QUAL_GOOD:
                color = 32;
                break;
            case DVB_QUAL_UNKNOWN:
            default:
                color = 0;
                break;
            }
            printf("\033[%dm", color);

            if (n_status_lines)
                printf("\t%s\n", buf);
            else
                printf("%s\n", buf);

            n_status_lines++;

            p = buf;
            len = sizeof(buf);
        }
    }

    return 0;
}

int main()
{
    auto dvb = dvb_dev_alloc();

    if (!dvb)
    {
        return 1;
    }

    auto find = dvb_dev_find(dvb, NULL, NULL);

    if (find)
    {
        printf("ERROR %d\n", errno);
    }
    else
    {
        for (auto adapter = 0; adapter >= 0; adapter++)
        {
            for (auto frontend = 0;; frontend++)
            {
                auto dev = dvb_dev_seek_by_adapter(dvb, adapter, frontend, DVB_DEVICE_FRONTEND);

                if (!dev)
                {
                    if (!frontend)
                    {
                        adapter = -2;
                    }

                    break;
                }

                printf("(%d, %d)\n", adapter, frontend);
            }
        }

        auto dev = dvb_dev_seek_by_adapter(dvb, 0, 0, DVB_DEVICE_FRONTEND);

        if (dev)
        {
            auto open = dvb_dev_open(dvb, dev->sysname, O_RDWR);

            if (open)
            {
                //print_frontend_stats(dvb->fe_parms);

                dvb_dev_close(open);
            }
        }
    }

    dvb_dev_free(dvb);
}

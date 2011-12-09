#include <Global.h>
#include <Config.h>

bool Config::Load()
{
    char dest[200];

    //Globalni uzivatelsky config
    sprintf(dest,"%s/global.set",SETTINGS_PATH);
    FILE* config = fopen(dest,"r");

    //Pokud neexistuje, zkusit nacist defaultni config (nemenny, dodavany autory)
    if (!config)
    {
        sprintf(dest,"%s/default_global.set",SETTINGS_PATH);
        config = fopen(dest,"r");
    }

    //A pokud se nenacetl soubor configu, opustit
    if (!config)
        return false;

    //Nejake pracovni promenne
    char setting[50];
    char value[50];
    int ivalue = 0;
    unsigned int i,j;
    locale loc;

    while (fgets(dest, 200, config))
    {
        i = 0;
        while (dest[i] != '=' && dest[i] != '\n')
        {
            setting[i] = dest[i];
            i++;
        }
        setting[i] = '\0';
        i++;
        j = 0;
        while (dest[i] != '\n')
        {
            value[j] = dest[i];
            i++; j++;
        }
        value[j] = '\0';
        
        //Rozpoznani nastaveni
        for (int k = 0; k < strlen(setting); k++)
            setting[k] = std::toupper(setting[k],loc);

        ivalue = atoi(value);

        if (strcmp(setting,"WINDOW_WIDTH") == 0)
            if (ivalue > 0)
                WindowWidth = ivalue;

        if (strcmp(setting,"WINDOW_HEIGHT") == 0)
            if (ivalue > 0)
                WindowHeight = ivalue;

        if (strcmp(setting,"COLOR_DEPTH") == 0)
            if (ivalue > 0)
                ColorDepth = ivalue;

        if (strcmp(setting,"FULLSCREEN") == 0)
            fullscreen = ivalue?true:false;

        if (strcmp(setting,"REFRESH_RATE") == 0)
            if (ivalue > 0)
                RefreshRate = ivalue;

        if (strcmp(setting,"HOST") == 0)
            HostName = value;

        if (strcmp(setting,"PORT") == 0)
            if (ivalue > 0)
                NetworkPort = ivalue;
    }

    return true;
}

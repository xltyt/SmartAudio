/*
    Create Scaffold Net
 */
#include "esp.h"

static int forward(Edi *db) {
    ediAddTable(db, "net");
    ediAddColumn(db, "net", "id", EDI_TYPE_INT, EDI_AUTO_INC | EDI_INDEX | EDI_KEY);
    ediAddColumn(db, "net", "title", EDI_TYPE_STRING, 0);
    ediAddColumn(db, "net", "body", EDI_TYPE_TEXT, 0);
    return 0;
}

static int backward(Edi *db) {
    ediRemoveTable(db, "net");
    return 0;
}

ESP_EXPORT int esp_migration_create_scaffold_net(Edi *db)
{
    ediDefineMigration(db, forward, backward);
    return 0;
}

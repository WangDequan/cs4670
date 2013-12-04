#ifndef CONTROLS_BOX_H
#define CONTROLS_BOX_H

#include "Common.h"
#include "ParametersMap.h"

#include <FL/Fl_Table.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Light_Button.H>
#include <FL/fl_draw.H>

class FieldsTable : public Fl_Table
{
public:
    FieldsTable(int x, int y, int w, int h, const char *l = NULL);
    ~FieldsTable();

    void setFields(const ParametersMap &fields);
    ParametersMap getFieldValues() const;

    // Extending inherited methods
    void draw_cell(TableContext context, int row, int col, int x, int y, int width, int height);

private:
    std::vector<std::string> _fieldNames;
    ParametersMap _fields;
};

class ControlsBox : public Fl_Group
{
public:
    ControlsBox(int x, int y, int w, int h, const char *label = NULL,
                const char *buttonText = NULL,
                Fl_Callback *buttonCallback = NULL, void *callbackData = NULL);

    void setFields(const ParametersMap &fields);
    ParametersMap getFieldValues() const;

    using Fl_Group::deactivate;
    void deactivateFieldTable();

private:
    FieldsTable *_fieldTable;
};

#endif // CONTROLS_BOX_H
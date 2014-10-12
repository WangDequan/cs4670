#include "ControlsBox.h"

FieldsTable::FieldsTable(int x, int y, int w, int h, const char *l):
    Fl_Table(x, y, w, h, l)
{
    col_header(false);
    col_resize(true);
    row_header(true);
    row_resize(false);
    row_header_width(200);
    table_box(FL_NO_BOX);
    end();
}

FieldsTable::~FieldsTable()
{
}

void
cbInput(Fl_Widget *widget, void *data)
{
    Fl_Input *input = (Fl_Input *) widget;

    ParametersMap *fields = (ParametersMap *) data;

    const char *label = input->label();
    const char *value = input->value();

    (*fields)[label] = value;
}

void
FieldsTable::setFields(const ParametersMap &fields)
{
    _fields = fields;

    int nRows = _fields.size();

    // clear previous widgets, if any
    clear();

    rows(nRows);
    _fieldNames.resize(nRows);
    cols(1);

    begin();
    {
        ParametersMap::iterator field = _fields.begin();
        for(int r = 0; r < nRows; r++, field++) {
            int x, y, w, h;
            find_cell(CONTEXT_TABLE, r, 0, x, y, w, h);

            Fl_Input *in = new Fl_Input(x, y, w, h, field->first.c_str());
            in->callback(cbInput, &_fields);
            in->value(field->second.c_str());

            _fieldNames[r] = field->first;
        }
    }
    end();

    col_width(0, this->w() - row_header_width() - 5);
}

ParametersMap
FieldsTable::getFieldValues() const
{
    return _fields;
}

// Handle drawing all cells in table
void
FieldsTable::draw_cell(TableContext context,
                       int r, int c, int x, int y, int w, int h)
{

    switch ( context ) {
    case CONTEXT_STARTPAGE:
        fl_font(FL_HELVETICA, 12);        // font used by all headers
        break;

    case CONTEXT_RC_RESIZE: {
        int x, y, w, h;
        for ( int r = 0, index = 0; r < rows(); r++ ) {
            for ( int c = 0; c < cols() && index <= children(); c++, index++ ) {
                find_cell(CONTEXT_TABLE, r, c, x, y, w, h);
                child(index)->resize(x, y, w, h);
            }
        }
        init_sizes();         // tell group children resized
        return;
    }

    case CONTEXT_ROW_HEADER:
        //    fl_draw_box(FL_DOWN_BOX, this->x(), this->y(), this->w(), this->h(), FL_GRAY);
        fl_push_clip(x, y, w, h);
        {
            fl_draw_box(FL_FLAT_BOX, x, y, w, h, FL_GRAY);

            fl_color(FL_BLACK);
            char label[120];
            sprintf(label, "%s: ", _fieldNames[r].c_str());
            fl_draw(label, x, y, w, h, FL_ALIGN_RIGHT);
        }
        fl_pop_clip();
        return;

    case CONTEXT_COL_HEADER:
        return;

    case CONTEXT_CELL:
        // fltk handles drawing the widgets
        return;

    default:
        return;
    }
}

ControlsBox::ControlsBox(int x, int y, int w, int h, const char *label,
                         const char *buttonText,
                         Fl_Callback *buttonCallback, void *callbackData):
    Fl_Group(x, y, w, h)
{
    const int buttonHeight =  (buttonText != NULL) ? 40 : 00;
    _fieldTable = new FieldsTable(x, y, w, h - buttonHeight - DEFAULT_BORDER);

    if(buttonText != NULL) {
        Fl_Button *button = new Fl_Button(x, _fieldTable->h() + _fieldTable->y() + DEFAULT_BORDER, _fieldTable->w(), buttonHeight, buttonText);

        if(buttonCallback != NULL) button->callback(buttonCallback, (void *)callbackData);
    }

    end();
}

void
ControlsBox::setFields(const ParametersMap &fields)
{
    assert(_fieldTable != NULL);
    _fieldTable->setFields(fields);
}

ParametersMap
ControlsBox::getFieldValues() const
{
    assert(_fieldTable != NULL);
    return _fieldTable->getFieldValues();
}

void
ControlsBox::deactivateFieldTable()
{
    _fieldTable->deactivate();
}
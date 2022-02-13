#include "cell.h"

#include <cassert>
#include <iostream>
#include <optional>
#include <string>

/* CELL VALUE */

CellValueInterface::CellValueInterface(CellValueInterface::Type type) : type_(type) {}

CellValueInterface::Type CellValueInterface::GetType() const {
    return type_;
}

/* EMPTY CELL VALUE */

EmptyCellValue::EmptyCellValue() : CellValueInterface(CellValueInterface::Type::Empty) {}

CellValueInterface::Value EmptyCellValue::GetValue() const {
    return 0.;
}
CellValueInterface::Value EmptyCellValue::GetRawValue() const {
    return 0.;
}
std::string EmptyCellValue::GetText() const {
    using namespace std::string_literals;
    return ""s;
}

/* TEXT CELL VALUE */

TextCellValue::TextCellValue(std::string text)
    : CellValueInterface(CellValueInterface::Type::Text), text_(std::move(text)) {}

CellValueInterface::Value TextCellValue::GetValue() const {
    return (!text_.empty() && text_.front() == ESCAPE_SIGN) ? text_.substr(1) : text_;
}
CellValueInterface::Value TextCellValue::GetRawValue() const {
    return text_;
}
std::string TextCellValue::GetText() const {
    return text_;
}

/* FORMULA CELL VALUE */

struct ValueErrorConverter {
    CellInterface::Value operator()(double value) {
        return value;
    }

    CellValueInterface::Value operator()(FormulaError error) {
        return error;
    }
};

FormulaCellValue::FormulaCellValue(std::string text)
    : CellValueInterface(CellValueInterface::Type::Formula), formula_(ParseFormula(std::move(text))) {}

CellValueInterface::Value FormulaCellValue::GetValue() const {
    if (formula_)
        return std::visit(ValueErrorConverter{}, formula_->Evaluate());
    return FormulaError("");
}
CellValueInterface::Value FormulaCellValue::GetRawValue() const {
    return GetValue();
}
std::string FormulaCellValue::GetText() const {
    using namespace std::string_literals;
    return "="s + formula_->GetExpression();
}

// Реализуйте следующие методы
Cell::Cell() {}

void Cell::Set(std::string text) {
    using namespace std::string_literals;

    try {
        if (text.front() == FORMULA_SIGN && text.size() > 1) {
            value_ = std::make_unique<FormulaCellValue>(text.substr(1));
        } else if (!text.empty()) {
            value_ = std::make_unique<TextCellValue>(std::move(text));
        } else {
            value_ = std::make_unique<EmptyCellValue>();
        }
    } catch (std::exception& e) {
        throw FormulaException("Error in Set() method call for text: "s + text);
    }
}

void Cell::Clear() {
    value_ = nullptr;
}

Cell::Value Cell::GetValue() const {
    return value_->GetValue();
}
std::string Cell::GetText() const {
    return value_->GetText();
}
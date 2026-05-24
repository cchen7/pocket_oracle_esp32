// Home menu — 4 x 3 card grid covering all 12 V1 apps.
// Cursor right via shake or KEY1 short press selects next card; KEY1 long
// press would normally return to home, but the home menu IS home, so the
// router will simply re-enter it (no-op).
//
// Until the per-app implementations land, every non-home entry resolves
// to a "Coming soon" stub via make_stub_app(label).

#include "home_menu.h"

#include "../../app/app_router.h"
#include "../../ui/theme.h"
#include "../answer_book/answer_book.h"
#include "../decision/decision_apps.h"
#include "../ritual/ritual_apps.h"
#include "../tools/tool_apps.h"
#include "lvgl.h"

namespace pocket {

namespace {

// 12 apps, fixed display order; matches PRD §四 functional list.
struct AppEntry {
    const char* label;
    const char* glyph;  // single-glyph emoji-ish stand-in until icons land.
};

const AppEntry kEntries[] = {
    { "Answer",  "?"  },
    { "Coin",    "C"  },
    { "Dice",    "D"  },
    { "1..10",   "10" },
    { "Yes/No",  "Y/N"},
    { "MBTI",    "M"  },
    { "Fortune", "F"  },
    { "Clock",   "T"  },
    { "Muyu",    "*"  },
    { "BLE",     "B"  },
    { "Power",   "P"  },
    { "Setup",   "S"  },
};
constexpr int kCount = sizeof(kEntries) / sizeof(kEntries[0]);
constexpr int kCols  = 4;
constexpr int kRows  = 3;
static_assert(kRows * kCols == kCount, "grid size mismatch");

class HomeMenuApp final : public AppBase {
public:
    const char* name() const override { return "Home"; }

    void on_enter(lv_obj_t* root) override
    {
        lv_obj_set_style_pad_all(root, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        // Grid container fills the content band.
        grid_ = lv_obj_create(root);
        lv_obj_remove_style_all(grid_);
        lv_obj_set_size(grid_, theme::SCREEN_W - 2 * theme::SPACE_S,
                              theme::CONTENT_H - theme::SPACE_S);
        lv_obj_align(grid_, LV_ALIGN_TOP_MID,
                     0, theme::CONTENT_TOP + theme::SPACE_XS);
        lv_obj_set_style_bg_opa(grid_, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_pad_row(grid_, theme::SPACE_XS, LV_PART_MAIN);
        lv_obj_set_style_pad_column(grid_, theme::SPACE_XS, LV_PART_MAIN);

        static int32_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                    LV_GRID_FR(1), LV_GRID_FR(1),
                                    LV_GRID_TEMPLATE_LAST};
        static int32_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                    LV_GRID_FR(1),
                                    LV_GRID_TEMPLATE_LAST};
        lv_obj_set_grid_dsc_array(grid_, col_dsc, row_dsc);
        lv_obj_set_layout(grid_, LV_LAYOUT_GRID);

        for (int i = 0; i < kCount; ++i) {
            lv_obj_t* card = lv_obj_create(grid_);
            lv_obj_remove_style_all(card);
            lv_obj_set_style_bg_color(card, theme::bg_elevated(), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(card, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_radius(card, 6, LV_PART_MAIN);
            lv_obj_set_style_border_width(card, 2, LV_PART_MAIN);
            lv_obj_set_style_border_color(card, theme::bg_elevated(), LV_PART_MAIN);
            lv_obj_set_grid_cell(card,
                                 LV_GRID_ALIGN_STRETCH, i % kCols, 1,
                                 LV_GRID_ALIGN_STRETCH, i / kCols, 1);

            lv_obj_t* glyph = lv_label_create(card);
            lv_label_set_text(glyph, kEntries[i].glyph);
            lv_obj_set_style_text_color(glyph, theme::ink_primary(), LV_PART_MAIN);
            lv_obj_set_style_text_font(glyph, theme::font_title(), LV_PART_MAIN);
            lv_obj_align(glyph, LV_ALIGN_TOP_MID, 0, 2);

            lv_obj_t* label = lv_label_create(card);
            lv_label_set_text(label, kEntries[i].label);
            lv_obj_set_style_text_color(label, theme::ink_secondary(), LV_PART_MAIN);
            lv_obj_set_style_text_font(label, theme::font_caption(), LV_PART_MAIN);
            lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -2);

            cards_[i] = card;
        }

        repaint_selection();
    }

    void on_event(InputEvent ev) override
    {
        switch (ev) {
            case InputEvent::kShake:
                cursor_ = (cursor_ + 1) % kCount;
                repaint_selection();
                break;
            case InputEvent::kButtonShortPress: {
                std::unique_ptr<AppBase> next;
                switch (cursor_) {
                    case 0: next = make_answer_book_app(); break;
                    case 1: next = make_coin_app();        break;
                    case 2: next = make_dice_app();        break;
                    case 3: next = make_random10_app();    break;
                    case 4: next = make_yesno_app();       break;
                    case 5: next = make_mbti_app();        break;
                    case 6: next = make_fortune_app();     break;
                    case 7: next = make_clock_app();       break;
                    case 8: next = make_muyu_app();        break;
                    default: next = make_stub_app(kEntries[cursor_].label);
                }
                app_router_push(std::move(next));
                break;
            }
            default:
                break;
        }
    }

    void on_exit() override
    {
        // root is wiped by the router; nothing to free.
    }

private:
    void repaint_selection()
    {
        for (int i = 0; i < kCount; ++i) {
            lv_color_t border = (i == cursor_) ? theme::accent_main()
                                               : theme::bg_elevated();
            lv_obj_set_style_border_color(cards_[i], border, LV_PART_MAIN);
        }
    }

    static std::unique_ptr<AppBase> make_stub_app(const char* label);

    lv_obj_t* grid_              = nullptr;
    lv_obj_t* cards_[kCount]     = {nullptr};
    int       cursor_            = 0;
};

// Generic placeholder used by every app entry that isn't built yet.
class StubApp final : public AppBase {
public:
    explicit StubApp(const char* label) : label_(label) {}
    const char* name() const override { return label_; }

    void on_enter(lv_obj_t* root) override
    {
        lv_obj_set_style_bg_color(root, theme::bg_primary(), LV_PART_MAIN);

        lv_obj_t* title = lv_label_create(root);
        lv_label_set_text(title, label_);
        lv_obj_set_style_text_color(title, theme::accent_main(), LV_PART_MAIN);
        lv_obj_set_style_text_font(title, theme::font_title(), LV_PART_MAIN);
        lv_obj_align(title, LV_ALIGN_CENTER, 0, -8);

        lv_obj_t* sub = lv_label_create(root);
        lv_label_set_text(sub, "Coming soon");
        lv_obj_set_style_text_color(sub, theme::ink_secondary(), LV_PART_MAIN);
        lv_obj_set_style_text_font(sub, theme::font_caption(), LV_PART_MAIN);
        lv_obj_align(sub, LV_ALIGN_CENTER, 0, 14);
    }

private:
    const char* label_;
};

std::unique_ptr<AppBase> HomeMenuApp::make_stub_app(const char* label)
{
    return std::make_unique<StubApp>(label);
}

}  // namespace

std::unique_ptr<AppBase> make_home_menu()
{
    return std::make_unique<HomeMenuApp>();
}

}  // namespace pocket

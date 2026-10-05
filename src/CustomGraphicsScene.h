#pragma once
#include <qgraphicsscene.h>

#include "CustomGraphicsItem.h"

class CustomGraphicsScene :
    public QGraphicsScene
{
    Q_OBJECT
public:
    CustomGraphicsScene() {};
    enum INSERT_DIRECTION { LEFT, RIGHT, UP, BOTTOM };
    enum EDIT_TYPE
    {
        NONE,
        SIZE_ADJUST_NODE,
        MOVE_NODE,
        ADD_TEXT,
        ADD_PADDING,
        REMOVE_NODE,
        HORIZONTAL_ALIGN,
        VERTICAL_ALIGN
    };
    enum PAINT_TYPE {CONTENT,NO_CONTENT,SCORE};
    enum TITLE_ROTATE {ROTATE_0, ROTATE_90, ROTATE_180, ROTATE_270};
    enum TITLE_ALIGN_H {LEFT_ALIGN, CENTER_ALIGN, RIGHT_ALIGN};
    enum TITLE_ALIGN_V {TOP_ALIGN, MIDDLE_ALIGN, BOTTOM_ALIGN};


    void SetEditType(EDIT_TYPE type)
    {
        this->edit_type_ = type;
    }

    EDIT_TYPE GetEditType()
    {
        return this->edit_type_;
    }

    void SetPaintType(PAINT_TYPE paint_type)
    {
        this->paint_type_ = paint_type;
    }

    PAINT_TYPE GetPaintType()
    {
        return this->paint_type_;
    }

    double GetVPadding()
    {
        return this->v_padding_;
    }

    double GetHPadding()
    {
        return this->h_padding_;
    }

    void SetVPadding(double padding)
    {
        this->v_padding_ = padding;
    }

    void SetHPadding(double padding)
    {
        this->h_padding_ = padding;
    }

    CustomGraphicsItem* GetSourceItem()
    {
        return this->source_item_;
    }

    CustomGraphicsItem* GetTargetItem()
    {
        return this->target_item_;
    }

    INSERT_DIRECTION GetInsertDirection()
    {
        if (item_helper_map_[target_item_][0] == target_rect_) return LEFT;
        if (item_helper_map_[target_item_][1] == target_rect_) return RIGHT;
        if (item_helper_map_[target_item_][2] == target_rect_) return UP;
        if (item_helper_map_[target_item_][3] == target_rect_) return BOTTOM;
        //return -1;
    }

    

    void GenerateHelperRects();
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void paint(QPaintEvent* event) ;

 signals:
        void ReOptimizeReady();

public  slots:
    void NodeEditDone();

private:
    EDIT_TYPE edit_type_ = NONE;
    PAINT_TYPE paint_type_ = PAINT_TYPE::CONTENT;
    PAINT_TYPE origin_paint_type_ = PAINT_TYPE::CONTENT;
    //move node
    double v_padding_;
    double h_padding_;
    //std::map<QRect, CustomGraphicsItem*> item_helper_map_;
	std::map<CustomGraphicsItem*, std::vector<QRect>> item_helper_map_;
    CustomGraphicsItem* source_item_ = nullptr;
    CustomGraphicsItem* move_item_ = nullptr;
	CustomGraphicsItem* target_item_ = nullptr;
    QRect target_rect_;
	QGraphicsRectItem* target_rect_item_ = nullptr;
    QGraphicsRectItem* delete_rect_item_ = nullptr;
    bool dragging_ = false;

    QFont title_font_;
    QFont text_font_;

    std::vector<CCombineTreeNode::AlignPair*> align_pair_list_;
    QGraphicsLineItem* current_line_ = nullptr;
    CCombineTreeNode::AlignPair* current_align_pair_ = nullptr;


public:
    const QFont &getTitleFont() const;

    void setTitleFont(const QFont &titleFont);

    const QFont &getTextFont() const;

    void setTextFont(const QFont &textFont);

    TITLE_ROTATE getTitleRotateValue() const;

    void setTitleRotateValue(TITLE_ROTATE titleRotateValue);

    TITLE_ALIGN_H getTitleAlignH() const;

    void setTitleAlignH(TITLE_ALIGN_H titleAlignH);

    TITLE_ALIGN_V getTitleAlignV() const;

    void setTitleAlignV(TITLE_ALIGN_V titleAlignV);

private:
    TITLE_ROTATE title_rotate_value_;
    TITLE_ALIGN_H title_align_h;
    TITLE_ALIGN_V title_align_v;




};


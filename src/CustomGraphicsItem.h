#pragma once
#include <qfont.h>
#include <QGraphicsRectItem>
#include <QGraphicsSceneHoverEvent>
#include <QToolBar>
#include "CCombineTreeNode.h"

class CustomGraphicsItem :
public QObject,
    public QGraphicsRectItem
{
    Q_OBJECT
public:
    CustomGraphicsItem();
    ~CustomGraphicsItem();
	
private:
    NODE_PRESENT_TYPE node_type_;
	//bool view_content_ = true;
	//text
	std::vector<std::pair<double, double>> start_point_list_;
    std::vector<QString> text_list_;
    //picture
    QPixmap pixmap_;
    //title_
    QString title_;

	CCombineTreeNode* tree_node_ = nullptr;
	int page_idx_;
    //size handle
    std::vector<QRect> size_handle_rects_;


    bool size_handle_ = false;
//    bool horizontal_align_handle_ = false;
//    bool vertical_align_handle_ = false;
    bool picture_show_lock_ = false;
    QRect picture_lock_icon_rect_ = QRect(- 14, -14, 28, 28);
    bool fix_geometry_ = false;
    //move node



public:
    //void SetViewContent(bool view_content) { view_content_ = view_content; }

    std::vector<QRect> horizontal_align_rects_;
    std::vector<QRect> vertical_align_rects_;

    void UpdateSizeHandleRects();
    void UpdateHorizontalAlignRects();
    void UpdateVerticalAlignRects();

    void setPos(qreal x, qreal y)
    {
        this->QGraphicsRectItem::setPos(x, y);
        UpdateSizeHandleRects();
        UpdateHorizontalAlignRects();
        UpdateVerticalAlignRects();
    };

    void setRect(qreal x, qreal y, qreal w, qreal h)
    {
        this->QGraphicsRectItem::setRect(x, y, w, h);
        UpdateSizeHandleRects();
        UpdateHorizontalAlignRects();
        UpdateVerticalAlignRects();
    };

	void SetTreeNode(CCombineTreeNode* node)
    {
        this->tree_node_ = node;
    }

    int GetPageIdx()
	{
        return this->page_idx_;
	}

    void SetPageIdx(int idx)
	{
        this->page_idx_ = idx;
	}

    CCombineTreeNode* GetTreeNode()
	{
        return this->tree_node_;
	}

    std::vector<std::pair<double, double>> GetStartPointList() const
    {
	    return start_point_list_;
    }

    void SetStartPointList(const std::vector<std::pair<double, double>>& start_point_list)
    {
	    start_point_list_ = start_point_list;
    }

    std::vector<QString> GetTextList() const
    {
	    return text_list_;
    }

    void SetTextList(const std::vector<QString> &text_list)
    {
	    text_list_ = text_list;
    }


    QPixmap GetPixmap() const
    {
	    return pixmap_;
    }

    void SetPixmap(const QPixmap& pixmap)
    {
	    this->pixmap_ = pixmap;
    }

    QString GetTitle() const
    {
	    return title_;
    }

    void SetTitle(const QString& title)
    {
	    this->title_ = title;
    }

    NODE_PRESENT_TYPE GetNodeType() const
    {
	    return node_type_;
    }

    void SetNodeType(NODE_PRESENT_TYPE node_type)
    {
	    this->node_type_ = node_type;
    }

    bool IsFixGeometry()
	{
        return this->fix_geometry_;
	}

    void SetFixGeometry(bool fix)
	{
        this->fix_geometry_ = fix;
	}


    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) Q_DECL_OVERRIDE;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
    void DrawSizeHandleRects(QPainter* painter);
    void DrawHorizontalRects(QPainter* painter);
    void DrawVerticalRects(QPainter* painter);
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

    signals:
        void  EditDone();

private:
    const std::vector<QColor>  LABEL_COLOR_ = {
	QColor(220, 220, 170),  // title_
	QColor(78, 201, 176),  // text
	QColor(86, 156, 198),  // picture
	QColor(154, 154, 154),  // padding
	QColor(255,255,255) //non-label
	};

    const std::vector<QColor>  LABEL_SCORE_COLOR_ = {
            QColor(255, 0, 0),  // title_
            QColor(0, 255, 0),  // text
            QColor(0, 0, 255),  // picture
            QColor(0, 0, 0),  // padding
            QColor(255,255,255) //non-label
    };



	const std::vector<QString>  LAYOUT_ICON_ =
	{
		":/LayoutShop/resources/layout-icon/header-white.svg",  // title_
		":/LayoutShop/resources/layout-icon/text-white.svg",  // text
		":/LayoutShop/resources/layout-icon/image.svg",  // picture
		nullptr,  // padding
		nullptr //non-label
	};
    const QString LOCK_ICON_ = ":/LayoutShop/resources/ui-icon/lock.svg";
    const QString UNLOCK_ICON_ = ":/LayoutShop/resources/ui-icon/unlock.svg";



    const std::vector<Qt::CursorShape> CURSOR_SHAPE_ = { Qt::SizeFDiagCursor, Qt::SizeVerCursor, Qt::SizeBDiagCursor,
        Qt::SizeHorCursor ,Qt::SizeHorCursor ,
    	Qt::SizeBDiagCursor, Qt::SizeVerCursor , Qt::SizeFDiagCursor };

    enum SIZE_ADJUST_DIRECTION { LEFT_TOP, MIDDLE_TOP, RIGHT_TOP, LEFT_CENTER, RIGHT_CENTER, LEFT_BOTTOM, MIDDLE_BOTTOM, RIGHT_BOTTOM, NONE_DIRECTION };
//    enum HORIZONTAL_ALIGN_EDGE { TOP, MIDDLE, BOTTOM, NONE_HORIZONTAL_ALIGN };
//    enum VERTICAL_ALIGN_EDGE { LEFT, CENTER,  RIGHT, NONE_VERTICAL_ALIGN };


    SIZE_ADJUST_DIRECTION size_adjust_direction_ = SIZE_ADJUST_DIRECTION::NONE_DIRECTION;
//    HORIZONTAL_ALIGN_EDGE horizontal_align_edge_;
//    VERTICAL_ALIGN_EDGE vertical_align_edge_;


};

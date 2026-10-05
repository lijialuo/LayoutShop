#include "ResultPreview.h"

#include "CustomGraphicsItem.h"

ResultPreview::ResultPreview(QWidget* widget)
{
}

ResultPreview::ResultPreview()
{
}

ResultPreview::~ResultPreview()
{
}

void ResultPreview::AddPreview(QGraphicsScene* scene)
{
	preview_list_.push_back(scene);
}

QGraphicsScene* ResultPreview::GetPreview(int index)
{
	if(index >= 0 && index < preview_list_.size())
	{
		return preview_list_[index];
	}
	return nullptr;
}

void ResultPreview::ClearAllPreview()
{
	for(auto preview:preview_list_)
	{
		for(auto item:preview->items())
		{
			delete item;
		}
		delete preview;
	}
	preview_list_.clear();
}

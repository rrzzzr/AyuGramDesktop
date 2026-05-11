#pragma once

#include "info/info_memento.h"
#include "info/media/info_media_widget.h"
#include "ui/object_ptr.h"

namespace Info::Forward {

class InnerWidget;

class Memento final : public ContentMemento {
public:
	Memento(not_null<Controller*> controller);
	Memento(not_null<UserData*> self);
	~Memento() override;

	Section section() const override;
	object_ptr<ContentWidget> createWidget(QWidget *parent, not_null<Controller*> controller, const QRect &geometry) override;

	[[nodiscard]] Media::Memento &media() {
		return _media;
	}
	[[nodiscard]] const Media::Memento &media() const {
		return _media;
	}

private:
	Media::Memento _media;
};

object_ptr<Info::Memento> Make(not_null<UserData*> self);

class Widget final : public ContentWidget {
public:
	Widget(QWidget *parent, not_null<Controller*> controller);
	~Widget() override;

	bool showInternal(not_null<ContentMemento*> memento) override;
	void setInternalState(const QRect &geometry, not_null<Memento*> memento) override;
	std::shared_ptr<ContentMemento> doCreateMemento() override;
	void saveState(not_null<Memento*> memento) override;
	void restoreState(not_null<Memento*> memento) override;

	rpl::producer<SelectedItems> selectedListValue() const override;
	void selectionAction(SelectionAction action) override;
	void fillTopBarMenu(const Ui::Menu::MenuCallback &addAction) override;

	rpl::producer<QString> title();

private:
	InnerWidget *_inner = nullptr;
};

} // namespace Info::Forward

#include "info/forward/info_forward_widget.h"

#include "info/forward/info_forward_inner_widget.h"
#include "info/info_controller.h"
#include "ui/boxes/confirm_box.h"
#include "ui/widgets/menu/menu_add_action_callback.h"
#include "ui/widgets/scroll_area.h"
#include "ui/ui_utility.h"
#include "core/application.h"
#include "data/data_download_manager.h"
#include "lang/lang_keys.h"
#include "styles/style_info.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "ayu/features/forward/ayu_forward.h"

#include <algorithm>

namespace Info::Forward {

Memento::Memento(not_null<Controller*> controller)
: ContentMemento(Tag{})
, _media(controller) {
}

Memento::Memento(not_null<UserData*> self)
: ContentMemento(Tag{})
, _media(self, 0, Media::Type::File) {
}

Memento::~Memento() = default;

Section Memento::section() const {
	return Section(Section::Type::Downloads);
}

object_ptr<ContentWidget> Memento::createWidget(
	QWidget *parent,
	not_null<Controller*> controller,
	const QRect &geometry) {
	auto result = object_ptr<Widget>(parent, controller);
	result->setInternalState(geometry, this);
	return result;
}

Widget::Widget(QWidget *parent, not_null<Controller*> controller)
: ContentWidget(parent, controller) {
	controller->setDownloadsFilter([](FullMsgId id) {
		return AyuForward::isForwardDownloadItem(id);
	});
	_inner = setInnerWidget(object_ptr<InnerWidget>(this, controller));
	_inner->setScrollHeightValue(scrollHeightValue());
	_inner->scrollToRequests(
	) | rpl::on_next([this](Ui::ScrollToRequest request) {
		scrollTo(request);
	}, _inner->lifetime());
}

Widget::~Widget() {
	controller()->setDownloadsFilter({});
}

bool Widget::showInternal(not_null<ContentMemento*> memento) {
	if (auto forwardMemento = dynamic_cast<Memento*>(memento.get())) {
		restoreState(forwardMemento);
		return true;
	}
	return false;
}

void Widget::setInternalState(const QRect &geometry, not_null<Memento*> memento) {
	setGeometry(geometry);
	Ui::SendPendingMoveResizeEvents(this);
	restoreState(memento);
}

std::shared_ptr<ContentMemento> Widget::doCreateMemento() {
	auto result = std::make_shared<Memento>(controller());
	saveState(result.get());
	return result;
}

void Widget::saveState(not_null<Memento*> memento) {
	memento->setScrollTop(scrollTopSave());
	_inner->saveState(memento);
}

void Widget::restoreState(not_null<Memento*> memento) {
	_inner->restoreState(memento);
	scrollTopRestore(memento->scrollTop());
}

rpl::producer<SelectedItems> Widget::selectedListValue() const {
	return _inner->selectedListValue();
}

void Widget::selectionAction(SelectionAction action) {
	_inner->selectionAction(action);
}

void Widget::fillTopBarMenu(const Ui::Menu::MenuCallback &addAction) {
	const auto window = controller()->parentController();
	const auto deleteAll = [=] {
		auto &manager = Core::App().downloadManager();
		auto ids = std::vector<GlobalMsgId>();
		auto allInCloud = true;
		const auto &filter = controller()->downloadsFilter();
		for (const auto id : manager.loadingList()) {
			if (filter && !filter(id->object.item->fullId())) {
				continue;
			}
			if (!id->object.item->isHistoryEntry()) {
				allInCloud = false;
			}
			ids.push_back(id->object.item->globalId());
		}
		for (const auto id : manager.loadedList()) {
			if (filter && !filter(id->object->item->fullId())) {
				continue;
			}
			if (!id->object->item->isHistoryEntry()) {
				allInCloud = false;
			}
			ids.push_back(id->object->item->globalId());
		}
		if (ids.empty()) {
			return;
		}
		std::sort(begin(ids), end(ids));
		ids.erase(std::unique(begin(ids), end(ids)), end(ids));
		const auto count = ids.size();
		const auto phrase = (count == 1)
			? tr::lng_downloads_delete_sure_one(tr::now)
			: tr::lng_downloads_delete_sure(tr::now, lt_count, int(count));
		const auto added = allInCloud
			? tr::lng_downloads_delete_in_cloud(tr::now)
			: QString();
		const auto deleteSure = [=, &manager](Fn<void()> close) {
			Ui::PostponeCall(this, close);
			manager.deleteFiles(ids);
		};
		window->show(Ui::MakeConfirmBox({
			.text = phrase + (added.isEmpty() ? QString() : "\n\n" + added),
			.confirmed = deleteSure,
			.confirmText = tr::lng_box_delete(tr::now),
			.confirmStyle = &st::attentionBoxButton,
		}));
	};
	addAction(
		tr::lng_context_delete_all_files(tr::now),
		deleteAll,
		&st::menuIconDelete);
}

rpl::producer<QString> Widget::title() {
	return tr::lng_forward_header_short();
}

std::shared_ptr<Info::Memento> Make(not_null<UserData*> self) {
	return std::make_shared<Info::Memento>(
		std::vector<std::shared_ptr<ContentMemento>>(
			1,
			std::make_shared<Memento>(self)));
}

} // namespace Info::Forward

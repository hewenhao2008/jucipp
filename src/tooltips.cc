#include "tooltips.h"
#include "singletons.h"

namespace sigc {
  SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

Gdk::Rectangle Tooltips::drawn_tooltips_rectangle=Gdk::Rectangle();

Tooltip::Tooltip(std::function<Glib::RefPtr<Gtk::TextBuffer>()> create_tooltip_buffer, Gtk::TextView& text_view, 
Glib::RefPtr<Gtk::TextBuffer::Mark> start_mark, Glib::RefPtr<Gtk::TextBuffer::Mark> end_mark):
create_tooltip_buffer(create_tooltip_buffer), text_view(text_view), 
start_mark(start_mark), end_mark(end_mark) {}

Tooltip::~Tooltip() {
  text_view.get_buffer()->delete_mark(start_mark);
  text_view.get_buffer()->delete_mark(end_mark);
}

void Tooltip::update() {
  auto iter=start_mark->get_iter();
  auto end_iter=end_mark->get_iter();
  text_view.get_iter_location(iter, activation_rectangle);
  if(iter.get_offset()<end_iter.get_offset()) {
    iter++;
    while(iter!=end_iter) {
      Gdk::Rectangle rectangle;
      text_view.get_iter_location(iter, rectangle);
      activation_rectangle.join(rectangle);
      iter++;
    }
  }
  int location_window_x, location_window_y;
  text_view.buffer_to_window_coords(Gtk::TextWindowType::TEXT_WINDOW_TEXT, activation_rectangle.get_x(), activation_rectangle.get_y(), location_window_x, location_window_y);
  activation_rectangle.set_x(location_window_x);
  activation_rectangle.set_y(location_window_y);
}

void Tooltip::adjust(bool disregard_drawn) {
  if(!window) {
    //init window
    window=std::unique_ptr<Gtk::Window>(new Gtk::Window(Gtk::WindowType::WINDOW_POPUP));
    
    window->set_events(Gdk::POINTER_MOTION_MASK);
    window->signal_motion_notify_event().connect([this](GdkEventMotion* event){
      window->hide();
      return false;
    });
    window->property_decorated()=false;
    window->set_accept_focus(false);
    window->set_skip_taskbar_hint(true);
    window->set_default_size(0, 0);

    tooltip_widget=std::unique_ptr<Gtk::TextView>(new Gtk::TextView(create_tooltip_buffer()));
    wrap_lines(tooltip_widget->get_buffer());
    tooltip_widget->set_editable(false);
    tooltip_widget->override_background_color(Gdk::RGBA(Singleton::Config::source()->background_tooltips)); 
    window->add(*tooltip_widget);

    auto layout=Pango::Layout::create(tooltip_widget->get_pango_context());
    layout->set_text(tooltip_widget->get_buffer()->get_text());
    layout->get_pixel_size(tooltip_width, tooltip_height);
    tooltip_height+=2;
  }
  
  int root_x, root_y;
  text_view.get_window(Gtk::TextWindowType::TEXT_WINDOW_TEXT)->get_root_coords(activation_rectangle.get_x(), activation_rectangle.get_y(), root_x, root_y);
  Gdk::Rectangle rectangle;
  rectangle.set_x(root_x);
  rectangle.set_y(root_y-tooltip_height);
  rectangle.set_width(tooltip_width);
  rectangle.set_height(tooltip_height);
  
  if(!disregard_drawn) {
    if(Tooltips::drawn_tooltips_rectangle.get_width()!=0) {
      if(rectangle.intersects(Tooltips::drawn_tooltips_rectangle))
        rectangle.set_y(Tooltips::drawn_tooltips_rectangle.get_y()-tooltip_height);
      Tooltips::drawn_tooltips_rectangle.join(rectangle);
    }
    else
      Tooltips::drawn_tooltips_rectangle=rectangle;
  }

  window->move(rectangle.get_x(), rectangle.get_y());
}

void Tooltip::wrap_lines(Glib::RefPtr<Gtk::TextBuffer> text_buffer) {
  INFO("Tooltip::wrap_lines");
  auto iter=text_buffer->begin();
  
  while(iter) {
    auto last_space=text_buffer->end();
    bool end=false;
    for(unsigned c=0;c<=80;c++) {
      if(!iter) {
        end=true;
        break;
      }
      if(*iter==' ')
        last_space=iter;
      if(*iter=='\n') {
        end=true;
        iter++;
        break;
      }
      iter++;
    }
    if(!end) {
      while(!last_space && iter) { //If no space (word longer than 80)
        iter++;
        if(iter && *iter==' ')
          last_space=iter;
      }
      if(iter && last_space) {
        auto mark=text_buffer->create_mark(last_space);
        auto iter_mark=text_buffer->create_mark(iter);
        auto last_space_p=last_space++;
        text_buffer->erase(last_space, last_space_p);
        text_buffer->insert(mark->get_iter(), "\n");
        
        iter=iter_mark->get_iter();

        text_buffer->delete_mark(mark);
        text_buffer->delete_mark(iter_mark);
      }
    }
  }
}

void Tooltips::show(const Gdk::Rectangle& rectangle, bool disregard_drawn) {
  for(auto& tooltip: *this) {
    tooltip.update();
    if(rectangle.intersects(tooltip.activation_rectangle)) {
      tooltip.adjust(disregard_drawn);
      tooltip.window->show_all();
    }
    else if(tooltip.window)
      tooltip.window->hide();
  }
}

void Tooltips::show(bool disregard_drawn) {
  for(auto& tooltip: *this) {
    tooltip.update();
    tooltip.adjust(disregard_drawn);
    tooltip.window->show_all();
  }
}

void Tooltips::hide() {
  for(auto& tooltip: *this) {
    if(tooltip.window)
      tooltip.window->hide();
  }
}
#include "fileselector.h"

UIFileSelector::UIFileSelector(const std::string& title, const std::string& dir) : UIGroup(title, true) {
    listing   = new UIScrollLayout(vec2(520.0f, 100.0f));

    dir_path  = new UIDirSelectLabel(this, dir);
    file_path = new UIFileSelectLabel(this, "");

    
    filter_select = new UISelect();
    
    layout->addElement(new UILabelledElement("Path",  dir_path,  120.0f));
    layout->addElement(listing);
    layout->addElement(new UILabelledElement("Name",  file_path, 120.0f));
    layout->addElement(new UILabelledElement("Filter", filter_select, 120.0f));

    addFilter("All Files", "");
    
    updateListing();
}

bool _listing_sort (const boost::filesystem::path& a,const boost::filesystem::path& b) {
    bool dir_a = is_directory(a);
    bool dir_b = is_directory(b);

    if(dir_a != dir_b) return dir_b < dir_a;

    return boost::ilexicographical_compare(a.filename().string(), b.filename().string());
}

void UIFileSelector::addFilter(const std::string& name, const std::string& extension) {
    filter_select->addOption(name, extension);
}

bool UIFileSelector::changeDir(const boost::filesystem::path& dir) {

    if(!is_directory(dir)) return false;

    dir_path->setText(dir.string());

    updateListing();

    return true;
}

void UIFileSelector::updateListing() {

    if(dir_path->text.empty()) return;

    boost::filesystem::path p(dir_path->text);

    if(!is_directory(p)) return;

    std::vector<boost::filesystem::path> dir_listing;

    try {
        copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), back_inserter(dir_listing));
    } catch(const boost::filesystem::filesystem_error& exception) {
        // TODO: do something here?
        return;
    }
    listing->clear();

    std::sort(dir_listing.begin(), dir_listing.end(), _listing_sort);

    if(is_directory(p.parent_path())) {
        listing->addElement(new UIFileSelectorLabel(this, "..", p.parent_path()));
    }

    foreach(boost::filesystem::path l, dir_listing) {

        std::string filename(l.filename().string());

        if(filename.empty()) continue;
#ifdef _WIN32
        DWORD win32_attr = GetFileAttributes(l.string().c_str());
        if (win32_attr & FILE_ATTRIBUTE_HIDDEN || win32_attr & FILE_ATTRIBUTE_SYSTEM) continue;
#else
        if(filename[0] == '.') continue;
#endif

        listing->addElement(new UIFileSelectorLabel(this, l));
    }

    listing->update(0.1f);

    listing->horizontal_scrollbar->reset();
    listing->vertical_scrollbar->reset();

    listing->vertical_scrollbar->bar_step = 1.0f / listing->getElementCount();
}

//UIFileSelectorLabel

UIFileSelectorLabel::UIFileSelectorLabel(UIFileSelector* selector, const std::string& label, const boost::filesystem::path& path)
    : selector(selector), path(path), UILabel(label, false, false, 520.0f) {
    directory = is_directory(path);
}

UIFileSelectorLabel::UIFileSelectorLabel(UIFileSelector* selector, const boost::filesystem::path& path)
    : selector(selector), path(path), UILabel(path.filename().string(), false, false, 520.0f) {
    directory = is_directory(path);
}

void UIFileSelectorLabel::updateContent() {
    font_colour = selected  ? vec3(1.0f) :
                  directory ? vec3(0.0f, 1.0f, 1.0f) : vec3(0.0f, 1.0f, 0.0f);
    background = selected ? vec4(1.0f, 1.0f, 1.0f, 0.15f) : vec4(0.0f);
}

void UIFileSelectorLabel::doubleClick() {
    if(directory) {
        ui->deselect();
        selector->changeDir(path);
    }
}

//UIDirSelectLabel
UIDirSelectLabel::UIDirSelectLabel(UIFileSelector* selector, const std::string& dirname)
    : selector(selector), UILabel(dirname, true, false, 400.0f) {
}


void UIDirSelectLabel::submit() {
    selector->changeDir(text);
}

//UIFileSelectLabel
UIFileSelectLabel::UIFileSelectLabel(UIFileSelector* selector, const std::string& filename)
    : selector(selector), UILabel(filename, true, false, 400.0f) {
}

void UIFileSelectLabel::submit() {
    //if(selector->changeFilter(text)) return;

    //TODO: if this points to a file, prompt to replace file?
}

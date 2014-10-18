include $(top_srcdir)/m4/autotroll.mk

AUTOMAKE_OPTIONS = subdir-objects

AM_CFLAGS = -DDATA_PATH=\"$(datadir)\" -I$(top_srcdir)/src
AM_CPPFLAGS = -DDATA_PATH=\"$(datadir)\" -I$(top_srcdir)/src

bin_PROGRAMS = rsjobeditor

rsjobeditor_SOURCES  = \
 jobeditor/ui/jobeditor.ui \
 rsjobeditor.cc \
 jobeditor/ui/ExtendedTabWidget.cpp                    jobeditor/ui/ExtendedTabWidget.moc.cpp \
 jobeditor/ui/ExtendedTabWidgetContainerExtension.cpp  jobeditor/ui/ExtendedTabWidgetContainerExtension.moc.cpp \
 jobeditor/ui/ExtendedTabWidgetExtensionFactory.cpp    jobeditor/ui/ExtendedTabWidgetExtensionFactory.moc.cpp \
 jobeditor/ui/ExtendedTabWidgetPlugin.cpp              jobeditor/ui/ExtendedTabWidgetPlugin.moc.cpp \
 jobeditor/ui/TaskWidget.cpp                           jobeditor/ui/TaskWidget.moc.cpp \
 jobeditor/ui/SettingWidget.cpp                        jobeditor/ui/SettingWidget.moc.cpp \
 jobeditor/ui/SwitchWidget.cpp                         jobeditor/ui/SwitchWidget.moc.cpp \
 jobeditor/ui/ArgumentsModel.cpp                       jobeditor/ui/ArgumentsModel.moc.cpp \
 jobeditor/rsjobeditorapplication.cpp                  jobeditor/rsjobeditorapplication.moc.cpp \
 jobeditor/rsjobeditorapplication.h
rsjobeditor_CXXFLAGS = $(QT_CXXFLAGS) $(AM_CXXFLAGS)
rsjobeditor_CPPFLAGS = $(QT_CPPFLAGS) $(AM_CPPFLAGS)
rsjobeditor_LDFLAGS  = $(QT_LDFLAGS) $(LDFLAGS)
rsjobeditor_LDADD    = $(QT_LIBS) $(LDADD)

BUILT_SOURCES = \
 jobeditor/ui/jobeditor.ui.h \
 jobeditor/ui/ExtendedTabWidget.moc.cpp \
 jobeditor/ui/ExtendedTabWidgetContainerExtension.moc.cpp \
 jobeditor/ui/ExtendedTabWidgetExtensionFactory.moc.cpp \
 jobeditor/ui/ExtendedTabWidgetPlugin.moc.cpp \
 jobeditor/ui/SettingWidget.moc.cpp \
 jobeditor/ui/SwitchWidget.moc.cpp \
 jobeditor/ui/ArgumentsModel.moc.cpp \
 jobeditor/rsjobeditorapplication.moc.cpp 
 
clean-local:
	find . -name '*.moc.cpp' -exec rm {} \;
	find . -name '*.ui.h' -exec rm {} \;
	
#if BUILD_OS_IS_DARWIN
install-exec-hook:
	mkdir -p $(DESTDIR)$(bindir)/RSJobEditor.app
	mkdir -p $(DESTDIR)$(bindir)/RSJobEditor.app/Contents
	mkdir -p $(DESTDIR)$(bindir)/RSJobEditor.app/Contents/MacOS
	mkdir -p $(DESTDIR)$(bindir)/RSJobEditor.app/Contents/Resources
	mkdir -p $(DESTDIR)$(bindir)/RSJobEditor.app/Contents/Resources/English.lproj
	mkdir -p $(DESTDIR)$(bindir)/RSJobEditor.app/Contents/Frameworks
	cp jobeditor/Info.plist $(DESTDIR)$(bindir)/RSJobEditor.app/Contents
	echo -n 'APPL????' > $(DESTDIR)$(bindir)/RSJobEditor.app/Contents/PkgInfo
	touch $(DESTDIR)$(bindir)/RSJobEditor.app/Contents/MacOS/rsjobeditor
	rm $(DESTDIR)$(bindir)/RSJobEditor.app/Contents/MacOS/rsjobeditor
	ln -s $(DESTDIR)$(bindir)/rsjobeditor $(DESTDIR)$(bindir)/RSJobEditor.app/Contents/MacOS/rsjobeditor

uninstall-hook:
	rm -rf $(DESTDIR)$(bindir)/RSJobEditor.app
#endif
/*****************************************************************************
** Settings.cpp
** Author: Kraku
*****************************************************************************/
#include <QLabel>
#include <QLayout>
#include <QComboBox>
#include <QMessageBox>
#include <QSettings>
#include <QCoreApplication>
#include "Settings.h"
#include "Gui.h"
#include <math.h>

bool
  Settings::commanual = false;
alg_t
  Settings::algorythm = ALG16;
dap_t
  Settings::dap = TOGGLE;
bool
  Settings::showbbl = false;
speed_type
  Settings::speed = STANDARD;
QThread::Priority Settings::priority = QThread::NormalPriority;

Settings::Settings (QWidget * parent):QGroupBox (tr ("Settings"), parent) {
    QString dirPath = QCoreApplication::applicationDirPath();
    QSettings set(dirPath + "/GameBoyCartFlasher.conf", QSettings::IniFormat);
    //  QSettings set;

  labels = new QVBoxLayout ();
  combo_boxes = new QVBoxLayout ();
  down = new QHBoxLayout ();
  north = new QHBoxLayout ();
  all = new QVBoxLayout ();

  com_label = new QLabel ("PORT:", this);
  labels->addWidget (com_label);
  com_combo = new QComboBox (this);
  combo_boxes->addWidget (com_combo);

  com_combo->insertItem (0, "USB Device:0");
  com_combo->insertItem (1, "USB Device:1");
  com_combo->insertItem (2, "USB Device:2");
  com_combo->insertItem (3, "USB Device:3");
  com_combo->insertItem (4, "USB Device:4");

  mbc_label = new QLabel ("MBC:", this);

  labels->addWidget (mbc_label);
  mbc_combo = new QComboBox (this);
  combo_boxes->addWidget (mbc_combo);
  mbc_combo->insertItem (0, "MBC AUTO");
  mbc_combo->insertItem (1, "MBC1");
  mbc_combo->insertItem (2, "MBC2");
  mbc_combo->insertItem (3, "MBC3");
  mbc_combo->insertItem (4, "ROM ONLY");
  mbc_combo->insertItem (5, "MBC5");
  mbc_combo->insertItem (6, "RUMBLE");
  mbc_combo->insertItem (7, "CUBIC FLASH");

  flash_label = new QLabel ("FLASH:", this);
  labels->addWidget (flash_label);
  flash_combo = new QComboBox (this);
  combo_boxes->addWidget (flash_combo);
  flash_combo->insertItem (0, "32 KB");
  flash_combo->insertItem (1, "64 KB");
  flash_combo->insertItem (2, "128 KB");
  flash_combo->insertItem (3, "256 KB");
  flash_combo->insertItem (4, "512 KB");
  flash_combo->insertItem (5, "1024 KB");
  flash_combo->insertItem (6, "2048 KB");
  flash_combo->insertItem (7, "4096 KB");
  flash_combo->insertItem (8, "8192 KB");
  flash_combo->setCurrentIndex (4);

  ram_label = new QLabel ("RAM:", this);
  labels->addWidget (ram_label);
  ram_combo = new QComboBox (this);
  combo_boxes->addWidget (ram_combo);
  ram_combo->insertItem (0, "2 KB");
  ram_combo->insertItem (1, "8 KB");
  ram_combo->insertItem (2, "32 KB");
  ram_combo->insertItem (3, "128 KB");
  ram_combo->setCurrentIndex (1);
  
  lang_label = new QLabel (tr ("LANGUAGE:"), this);
  labels->addWidget (lang_label);
  lang_combo = new QComboBox (this);
  combo_boxes->addWidget (lang_combo);
  get_langs();

  auto_check = new QCheckBox (tr ("Check file size"), this);
  auto_check->setCheckState (Qt::Checked);
  down->addWidget (auto_check);

  north->addLayout (labels);
  north->addLayout (combo_boxes);
  all->addLayout (north);
  all->addLayout (down);
  setLayout (all);

  auto_size = true;
  mbc = MBCAUTO;
  ram_size = 8;
  flash_size = 512;
  com_name = com_combo->currentIndex();
  Gui::port_type = USB;

  //connect (com_combo, SIGNAL (activated (int)), this, SLOT (setCom (int)));
  connect (mbc_combo, SIGNAL (activated (int)), this, SLOT (setMbc (int)));
  connect (flash_combo, SIGNAL (activated (int)), this, SLOT (setFlash (int)));
  connect (ram_combo, SIGNAL (activated (int)), this, SLOT (setRam (int)));
  connect (lang_combo, SIGNAL (activated (const QString &)), this, SLOT (setLang (const QString &)));
  connect (auto_check, SIGNAL (stateChanged (int)), this, SLOT (setAuto (int)));
}

unsigned int Settings::getCom () {
  com_name = com_combo->currentIndex();
  return com_name;
}

void Settings::setCom (int index) {
  getCom();
  com_combo->setCurrentIndex (index);
  Gui::port_type = USB;
}

void Settings::setFlash (int index) {
  /* size = 32 * 2 ^ index */
  flash_size = 32 * (int) pow (2.0, (double) index);
}

void
Settings::setRam (int index)
{
  switch (index)
    {
    case 0:
      ram_size = 2;
      break;
    case 1:
      ram_size = 8;
      break;
    case 2:
      ram_size = 32;
      break;
    case 3:
      ram_size = 128;
      break;
    }

}

void
Settings::setMbc (int mbc_nr)
{
  mbc = (mbc_t) mbc_nr;
  switch (mbc)
    {
    case MBCAUTO:
      ram_types (4);
      flash_types (8);
      break;
    case MBC1:
      ram_types (3);
      flash_types (6);
      break;
    case MBC2:
      ram_types (0);
      flash_types (3);
      break;
    case MBC3:
      ram_types (3);
      flash_types (6);
      break;
    case ROMONLY:
      ram_types (0);
      flash_types (0);
      break;
    case MBC5:
      ram_types (4);
      flash_types (8);
      break;
    case RUMBLE:
      ram_types (3);
      flash_types (8);
      break;
    case CUBIC_FLASH:
      ram_types (0);
      flash_types (0);
      break;
  }

  if(mbc == CUBIC_FLASH)
      Settings::algorythm = ALG16WR;
  else
      Settings::algorythm = ALG16;

  setFlash (flash_combo->currentIndex ());
  setRam (ram_combo->currentIndex ());
  emit refresh_ram_buttons ();
}

//fill ram_combo depending on cart type
void
Settings::ram_types (int type)
{
  int ile = ram_combo->count ();
  for (int i = ile - 1; i >= 0; i--)
    ram_combo->removeItem (i);
  switch (type)
    {
    case 0:
      break;
    case 3:
      ram_combo->insertItem (0, "2 KB");
      ram_combo->insertItem (1, "8 KB");
      ram_combo->insertItem (2, "32 KB");
      break;
    case 4:
      ram_combo->insertItem (0, "2 KB");
      ram_combo->insertItem (1, "8 KB");
      ram_combo->insertItem (2, "32 KB");
      ram_combo->insertItem (3, "128 KB");
      break;
    }
}

void
Settings::flash_types (int type)
{
  int ile = flash_combo->count ();
  for (int i = ile - 1; i >= 0; i--)
    flash_combo->removeItem (i);

  for (int i = 0; i <= type; i++)
    {
      int pojemnosc = 32 * (int) pow (2.0, (double) i);
      flash_combo->insertItem (i, QString::number (pojemnosc) + " KB");
    }
}

void
Settings::get_langs() {
    QString dirPath = QCoreApplication::applicationDirPath();
    QSettings settings(dirPath + "/GameBoyCartFlasher.conf", QSettings::IniFormat);
    //  QSettings settings;
  int selectedIndex = 0;
  QString selectedLang = settings.value("selected_lang").toString();
  settings.beginGroup("lang");
  QStringList keys = settings.childKeys();

  for (int i = 0; i < keys.size(); ++i) {
  	lang_combo->insertItem(i,settings.value(keys.at(i)).toString());
	if(selectedLang == keys.at(i))
		selectedIndex = i;
  }
  settings.endGroup();
  lang_combo->setCurrentIndex(selectedIndex);

}

void
Settings::setLang (const QString & lang)
{
    QString dirPath = QCoreApplication::applicationDirPath();
    QSettings settings(dirPath + "/GameBoyCartFlasher.conf", QSettings::IniFormat);
    //  QSettings settings;
  settings.beginGroup("lang");
  QString langFileName = settings.value("selected_lang").toString();
  QStringList keys = settings.childKeys();
  for (int i = 0; i < keys.size(); ++i) {
	if(lang == settings.value(keys.at(i)).toString())
		langFileName = keys.at(i);
  }
  settings.endGroup();
  settings.setValue ("selected_lang", langFileName);

  QMessageBox::information (this, tr ("Change language"),
			    tr
			    ("After program restart language will be changed to: ")
			    + lang, QMessageBox::Ok);
}

void Settings::setAuto (int state) {
  if (state == Qt::Checked)
    auto_size = true;
  else
    auto_size = false;
}

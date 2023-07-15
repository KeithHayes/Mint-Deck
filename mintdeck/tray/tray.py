#!/home/kdog/anaconda3/bin/python

import gi
gi.require_version('Gtk', '3.0')
gi.require_version('AppIndicator3', '0.1')
import os
import sys
import subprocess
from gi.repository import Gtk as gtk, AppIndicator3 as appindicator
from gi.repository import GdkPixbuf

def main():
    icon_path = os.path.expanduser("~/mintdeck/tray/5ptstar.png")
    indicator = appindicator.Indicator.new("customtray", icon_path, appindicator.IndicatorCategory.APPLICATION_STATUS)
    indicator.set_status(appindicator.IndicatorStatus.ACTIVE)
    indicator.set_menu(menu())
    gtk.main()

def menu():
    menu = gtk.Menu()

    command_one = gtk.MenuItem(label='Notes')
    command_one.connect('activate', note)
    menu.append(command_one)

    command_two = gtk.MenuItem(label='Launch Mintdeck')
    command_two.connect('activate', launch_mintdeck)
    menu.append(command_two)

    command_three = gtk.MenuItem(label='Show / Hide Mintdeck')
    command_three.connect('activate', toggle)
    menu.append(command_three)    
    
    exittray = gtk.MenuItem(label='Quit')
    exittray.connect('activate', quit)
    menu.append(exittray)

    menu.show_all()
    return menu

def note(_):
    os.system("gedit $HOME/Documents/notes.txt")
    while gtk.events_pending():
        gtk.main_iteration()

def launch_mintdeck(_):
    script_dir = os.path.expanduser('~/mintdeck')
    os.chdir(script_dir)
    subprocess.Popen(['./mintdeck'])

def toggle(_):
    pipe_path = '/tmp/receivemintdeck'
    message = 'toggledeck'
    try:
        with open(pipe_path, 'w') as pipe:
            pipe.write(message)
    except OSError as e:
        print(f"Error writing to named pipe '{pipe_path}': {e}")

def quit(_):
    gtk.main_quit()

def show_output(output):
    dialog = gtk.Dialog(title="Command Output")
    dialog.add_button('Close', gtk.ResponseType.CLOSE)
    dialog.set_default_response(gtk.ResponseType.CLOSE)

    textview = gtk.TextView()
    textview.set_editable(False)
    textview.set_cursor_visible(False)

    buffer = textview.get_buffer()
    buffer.set_text('')

    lines = output.split('\n')
    for line in lines:
        iter = buffer.get_end_iter()
        buffer.insert(iter, '        ' + line + '   \n')

    scrolled_window = gtk.ScrolledWindow()
    scrolled_window.set_policy(gtk.PolicyType.AUTOMATIC, gtk.PolicyType.AUTOMATIC)
    scrolled_window.add(textview)

    dialog.vbox.pack_start(scrolled_window, True, True, 0)
    dialog.show_all()
    dialog.set_size_request(200, -1)
    dialog.run()
    dialog.destroy()

if __name__ == "__main__":
    main()


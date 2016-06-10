#!/bin/bash
#   GMAIL New Mail Notifier with message sender/subject preview and
#       notification of new mail
#   
#   Requires: curl xfce4-genmon-plugin notify-send
#
#   xfce4-genmon-plugin properties:
#       Command = path to and name of this script
#       Label = optional label before icon (better unchecked)
#       Period = Time in seconds to check for new emails (to run this script)
#       Default Font = (your choice)
#
##############################################
### configurable parameters (adjust to suit)

  # gmail identification
USERNAME="xxxxxxxx"
PASSWORD="xxxxxxxx"
  # timezone
TIMEZONE="Canada/Eastern"
  # icons
NOMAIL="/home/toz/.icons/mail.png"
NEWMAIL="/home/toz/.icons/mail-new.png"
NOTIFICATION_ICON="/home/toz/.icons/aol_mail.png"
  # your email application (activated on click)
EMAILAPP="thunderbird"

##############################################

##############################################
# don't change anything below
##############################################

# get and save the atom feed
curl -u "$USERNAME":"$PASSWORD" --silent "https://mail.google.com/mail/feed/atom"  > /tmp/.gmail

# get number of unread messages
num_messages=$(grep -oP "(?<=<fullcount>)[^<]+" /tmp/.gmail)

# get last checked time
last_checked=$(grep -oP "(?<=<modified>)[^<]+" /tmp/.gmail | TZ=$TIMEZONE date +'%r')

# get ids, senders and subjects
mapfile -t ids < <(grep -oP "(?<=<id>)[^<]+" /tmp/.gmail | awk -F":" '{print $3}')
mapfile -t names < <(grep -oP "(?<=<name>)[^<]+" /tmp/.gmail)
mapfile -t subjects < <(grep -oP "(?<=<title>)[^<]+" /tmp/.gmail | grep -v Gmail)

# prepare tooltip string
out=$(for (( i=0; i<$num_messages; i++ )); do echo "${names[i]} - ${subjects[i]}#";  done)
toolstr="$(echo $out | sed -e 's/\# /\n/g' | sed -e 's/\#//g')"

# check to see if there are new, new messages (only notify if something new has arrived)
new_msgs=0

if [ $num_messages -gt 0 ]; then
    if [ -a /tmp/.gmail.lastid ]; then
        if [ "${ids[0]}" != "$(cat /tmp/.gmail.lastid)" ]; then
            echo ${ids[0]} > /tmp/.gmail.lastid
            let new_msgs=1
        fi
    else
        echo ${ids[0]} > /tmp/.gmail.lastid  
        let new_msgs=1  
    fi
fi
     

##### genmon processing

#set default icon file to no new emails
ICON_FILE="$NOMAIL"

if [ $num_messages -gt 0 ]; then
    # set icon file to new emails image
    ICON_FILE="$NEWMAIL"
    if [ $new_msgs -eq 1 ]; then
        notify-send -i "$NOTIFICATION_ICON" "You have Mail" "<i>$num_messages new message(s).</i>"
    fi
fi

##### do the genmon
if [ $num_messages -gt 0 ]
then
    echo "<img>$ICON_FILE</img>
    <click>$EMAILAPP</click>
    <tool>$num_messages new message(s)
    
$toolstr

Last checked: $last_checked</tool>"
else
    echo "<img>$ICON_FILE</img>
    <click>$EMAILAPP</click>
    <tool>No new mail
    
Last checked: $last_checked</tool>"
fi

unset ids names subjects
exit 0


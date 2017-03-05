#!/bin/bash
#   Twitter Timeline Notifier with tooltip tweet display
#     - will send notification of all new tweets 
#     - tooltip will show new and (optionally) recent tweets to a max of NUM_TOOLTIP_TWEETS
#     - plugin will change icon if new tweets detected
#     - clicking on plugin will: 
#        option 1: execute configurable command
#        option 2: display the saved recent tweets timeline
#        option 3: compose and send a tweet (untested)
#   
#   Requires:  xfce4-genmon
#              t (https://github.com/sferik/t/blob/master/README.md)
#              yad
#
#  NOTE: you must set up t (command line twitter client) properly for this script to work. 
#        Instructions in link above.
#
#   xfce4-genmon-plugin properties:
#       Command = path to and name of this script
#       Label = optional label before icon (better unchecked)
#       Period = Time in seconds to check for new tweets (to run this script)
#       Default Font = (your choice)
#
##############################################
### configurable parameters

  # debug mode (logged to /tmp/twit-log if set to 1)
DEBUG=0

  # location of t
TWIT_CMD="/home/toz/.gem/ruby/2.4.0/bin/t"

  # number of tweets to show in tooltip
NUM_TOOLTIP_TWEETS=12

  # have tooltip display new tweets only (1=yes, 0=also display recents tweets)
TOOLTIP_NEW_TWEETS_ONLY=1

  # icons
NOTWEETS="/home/toz/.icons/t1.png"
NEWTWEETS="/home/toz/.icons/t2.png"

  # notify-send icon
NOTIFICATION_ICON="/home/toz/.icons/twitter.svg"

  # On-click action (what to do when plugin icon is clicked)
  #
  # 1. Run a program (open twitter in browser window)
#CLICK_ACTION="xdg-open https://www.twitter.com"

  # 2. display list of recent tweets in yad dialog
cat /tmp/.twit-all | awk '{ printf("%s %- 16s", $4, $5); out=$6; for(i=7;i<=NF;i++){out=out" "$i}; print out}' > /tmp/.twit-all-output
CLICK_ACTION="yad --title Recent\ Twitter\ Timeline --center --width=1200 --height=500 --text-info --show-uri --filename=/tmp/.twit-all-output"

  # 3. compose a new tweet (untested)
#TWEET=$(yad --title "Compose a new message..." --height=200 --width=300 --text-info --editable --wrap)
#CLICK_ACTION="t update '$TWEET'"

##############################################

##############################################
# don't change anything below
##############################################

# get last processed ID and timeline
if [ -s /tmp/.lastid ]
then
   LASTID=$(cat /tmp/.lastid)
   $TWIT_CMD timeline -lr -s $LASTID > /tmp/.twit
else
   $TWIT_CMD timeline -lr > /tmp/.twit
fi
               [ $DEBUG -eq 1 ] && echo "01 .lastid=$(cat /tmp/.lastid)" > /tmp/twit-log
               [ $DEBUG -eq 1 ] && echo "02 LASTID=$LASTID" >> /tmp/twit-log

sleep 1

# save the last processed ID
cat /tmp/.twit | tail -1 | awk '{print $1}' > /tmp/.lastid
if [ ! -s /tmp/.lastid ] 
then
   echo $LASTID > /tmp/.lastid
fi
               [ $DEBUG -eq 1 ] && echo "03 new.lastid=$(cat /tmp/.lastid)" >> /tmp/twit-log

# get number of new tweets
num_tweets=$(cat /tmp/.twit | wc -l)
               [ $DEBUG -eq 1 ] && echo "04 num_tweets=$num_tweets" >> /tmp/twit-log

# get the contents of the new tweets
mapfile -t ids < <(awk '{print $1}' /tmp/.twit)
mapfile -t from < <(awk '{print $5}' /tmp/.twit)
mapfile -t text < <(awk '{for(i=6;i<=NF;i++){printf "%s ", $i}; printf "\n"}' /tmp/.twit)
toolstr=$(for (( i=0; i<$num_tweets; i++ )); do echo "${from[i]} >> ${text[i]}";  echo ""; done)
               [ $DEBUG -eq 1 ] && echo "05 toolstr=$toolstr" >> /tmp/twit-log

##### set default plugin icon file and notify of new tweets
#set default icon file to no new emails
ICON_FILE="$NOTWEETS"
               [ $DEBUG -eq 1 ] && echo "06 ICON_FILE=$ICON_FILE" >> /tmp/twit-log

if [ $num_tweets -gt 0 ]; then
               [ $DEBUG -eq 1 ] && echo "07 num_tweets > 0" >> /tmp/twit-log
    # set icon file to new emails image
    ICON_FILE="$NEWTWEETS"
               [ $DEBUG -eq 1 ] && echo "08 ICON_FILE=$ICON_FILE" >> /tmp/twit-log
    for (( i=0; i<$num_tweets; i++ ))
    do 
               [ $DEBUG -eq 1 ] && echo "09 sending notification" >> /tmp/twit-log    
      notify-send -i "$NOTIFICATION_ICON" "$(echo ${from[i]} | sed -r 's/[&]+/&amp;/g')" "$(echo ${text[i]} | sed -r 's/[&]+/&amp;/g')"
    done
fi

# append current to the all file
cat /tmp/.twit /tmp/.twit-all > /tmp/.twit-tmp && mv /tmp/.twit-tmp /tmp/.twit-all

# get last checked time stamp
last_checked=$(date)

# prepare tooltip string (to show last NUM_TOOLTIP_TWEETS)
if [ -s /tmp/.twit-all ]
then
   mapfile -t from2 < <(awk '{print $5}' /tmp/.twit-all)
   mapfile -t text2 < <(awk '{for(i=6;i<=NF;i++){printf "%s ", $i}; printf "\n"}' /tmp/.twit-all)
   toolstr2=$(for (( i=$num_tweets; i<$NUM_TOOLTIP_TWEETS; i++ )); do if [ "${from2[i]}" != "" ]; then echo "${from2[i]} >> ${text2[i]}";  echo ""; fi; done)
fi 
               [ $DEBUG -eq 1 ] && echo "10 toolstr2=$toolstr2" >> /tmp/twit-log  

##### do the genmon
if [ $TOOLTIP_NEW_TWEETS_ONLY -eq 0 ]
then

if [ $num_tweets -gt 0 ]
then
                 [ $DEBUG -eq 1 ] && echo "11 genmon:num_tweets > 0" >> /tmp/twit-log

echo "<img>$ICON_FILE</img>
<click>$CLICK_ACTION</click>
<tool>New tweets:

$toolstr

Recent tweets:
    
$toolstr2

Last checked: $last_checked</tool>"

else
               [ $DEBUG -eq 1 ] && echo "12 genmon:num_tweets = 0" >> /tmp/twit-log

echo "<img>$ICON_FILE</img>
<click>$CLICK_ACTION</click>
<tool>Recent tweets:
    
$toolstr2

Last checked: $last_checked</tool>"

fi

else
   
if [ $num_tweets -gt 0 ]
then
                 [ $DEBUG -eq 1 ] && echo "11 genmon:num_tweets > 0" >> /tmp/twit-log

echo "<img>$ICON_FILE</img>
<click>$CLICK_ACTION</click>
<tool>New tweets:

$toolstr

Last checked: $last_checked</tool>"

else
               [ $DEBUG -eq 1 ] && echo "12 genmon:num_tweets = 0" >> /tmp/twit-log

echo "<img>$ICON_FILE</img>
<click>$CLICK_ACTION</click>
<tool>No new tweets.

Last checked: $last_checked</tool>"

fi
   
fi      

               [ $DEBUG -eq 1 ] && echo "13 $(date)" >> /tmp/twit-log

unset ids from text from2 text2
exit 0


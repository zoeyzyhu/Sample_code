#!/usr/bin/env python3.7

from bs4 import BeautifulSoup
from time import sleep
from random import randint
import urllib3
import requests
import hashlib
import argparse
import re

parser = argparse.ArgumentParser()
parser.add_argument('-o',
    help = 'output file'
    )
args = vars(parser.parse_args())


# connection pool and request making
http = urllib3.PoolManager()

for i in range(1,388):
    url = 'http://hotels.ctrip.com/hotel/shanghai2/p'
    url += str(i)
    if i % 10 == 0:
        sleep(randint(10, 20))
    else:
        sleep(randint(1, 10))
    print(url)
    response = http.request('GET', url)

# parse the html using beautiful soup and store in variable 'soup'
    soup = BeautifulSoup(response.data, "html.parser")

# get the whole page
#print(soup.prettify())

# get hotel names
    htlname = []
    p = re.compile('"name":"(.*?)"')
    htlname.extend(p.findall(str(soup)))

# get hotel ids
    htlid = []
    hshtlid = []
    for tag in soup.find_all('div', class_ = 'hotel_new_list J_HotelListBaseCell'):
        hid = tag.get('id')
        htlid.append(hid)
        hshtlid.append(hashlib.md5(hid.encode()).hexdigest())

# get hotel_strategymedal
    medal = []
    qmedal = []
    brandq = []
    star = []
    for ico in soup.find_all('span', attrs = {'class': 'hotel_ico'}):
        lmedal = []
        for tag in ico.find_all('span'):
            lmedal.append(tag.attrs['class'])
        if len(lmedal) == 4:
            medal.append(lmedal[0])
            brandq.append(lmedal[1])
            qmedal.append(lmedal[2])
            star.append(lmedal[3])
        if len(lmedal) == 3:
            medal.append(lmedal[0])
            brandq.append('null')
            qmedal.append(lmedal[1])
            star.append(lmedal[2])
        if len(lmedal) == 2:
            medal.append(lmedal[0])
            brandq.append('null')
            qmedal.append('null')
            star.append(lmedal[1])

# get hotel imagelink
    imglink = []
    for tag in soup.find_all('div', class_ = 'dpic J_as_bottom'):
        imglink.append(tag.img.get('src'))

# get hotel address
    loc = []
    for tag in soup.find_all('p', class_ = 'hotel_item_htladdress'):
        loc.append(tag.text)

# get specials
    special = []
    for tag in soup.find_all('span', class_ = 'special_label'):
        lspecial = []
        for item in tag.find_all('i'):
            lspecial.append(item.text)
        special.append(lspecial)

# get icon list
    icon = []
    for tag in soup.find_all('div', class_ = 'icon_list'):
        licon = []
        for item in tag.find_all('i'):
            licon.append(item.get('title'))
        icon.append(licon)

# get low price
    lowprice = []
    for tag in soup.find_all('span', class_ = 'J_price_lowList'):
        lowprice.append(tag.text)

# get hotel scores
    revscore = []
    revcount = []
    recscore = []
    p = re.compile('"score":"([0-9]*\.[0-9]*)"')
    revscore.extend(p.findall(str(soup)))
    p = re.compile('"dpscore":"([0-9]*)"')
    recscore.extend(p.findall(str(soup)))
    p = re.compile('"dpcount":"([0-9]*)"')
    revcount.extend(p.findall(str(soup)))
    reclevel = []
    reccomment = []
    for tag in soup.find_all('div', class_ = 'hotelitem_judge_box'):
        item1 = tag.find('span', class_ = 'recommend')
        item2 = tag.find('span', class_ = 'hotel_level')
        if item1:
            reccomment.append(item1.text)
        else:
            reccomment.append('null')
        if item2:
            reclevel.append(item2.text)
        else:
            reclevel.append('null')


# get latitude and longitude
    lat = []
    p = re.compile('"lat":"([0-9]*\.[0-9]*)"')
    lat.extend(p.findall(str(soup)))

    lon = []
    p = re.compile('"lon":"([0-9]*\.[0-9]*)"')
    lon.extend(p.findall(str(soup)))

# write lists into the output file
    f = open('%s' % args['o'], 'a')
    for j in range(len(htlid)):
        f.write('%s\t' % htlid[j])
        f.write('%s\t' % htlname[j])
        f.write('%s\t' % hshtlid[j])
        f.write('%s\t' % medal[j])
        f.write('%s\t' % brandq[j])
        f.write('%s\t' % qmedal[j])
        f.write('%s\t' % star[j])
        f.write('%f\t' % float(revscore[j]))
        f.write('%d\t' % int(revcount[j]))
        f.write('%d\t' % int(recscore[j]))
        f.write('%s\t' % reclevel[j])
        f.write('%s\t' % reccomment[j])
        f.write('%d\t' % int(lowprice[j]))
        f.write('%s\t' % special[j])
        f.write('%s\t' % icon[j])
        f.write('%f\t' % float(lat[j]))
        f.write('%f\t' % float(lon[j]))
        f.write('%s\t' % loc[j])
        f.write('%s\n' % imglink[j])
    f.close()

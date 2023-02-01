#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <getopt.h>

#define MAXLINE 65536
#define MAXNAME 2048
#define NCHOICE	19

char *binary_fmt(size_t x, char buf[NCHOICE+1]) {
	char *s = buf + NCHOICE;
	*--s = 0;
	if (!x) *--s = '0';
	for(; x; x/=2) *--s = '0' + x%2;
	return s;
}

int main(int argc, char *argv[]) {
	char line[MAXLINE];
	char uid[MAXNAME];
	char qid_new[MAXNAME];
	char starttime[MAXNAME];
	char hotelnum[MAXNAME];
	char hotelid[MAXNAME];
	char rank_in_qid[MAXNAME];
	char price_qry[MAXNAME];
	char star_qry[MAXNAME];
	char keyword_qry[MAXNAME];
	char cityid_qry[MAXNAME];
	char checkin[MAXNAME];
	char checkout[MAXNAME];
	char spec_qry[MAXNAME];
	char brand1_qry[MAXNAME];
	char brand2_qry[MAXNAME];
	char zone_qry[MAXNAME];
	char bookable_qry[MAXNAME];
	char dist_qry[MAXNAME];
	char fac_qry[MAXNAME];
	char filter_score[MAXNAME];
	char filter_quantity[MAXNAME];
	char com[MAXNAME];
	char sta[MAXNAME];
	char distr[MAXNAME];
	char metro1[MAXNAME];
	char metro2[MAXNAME];
	char hotd[MAXNAME];
	char sort_qry[MAXNAME];
	char bedtype[MAXNAME];
	char breakfast[MAXNAME];
	char paytype[MAXNAME];
	char fh_price[MAXNAME];
	char dist[MAXNAME];
	char isprom[MAXNAME];
	char isgift[MAXNAME];
	char iscoupon[MAXNAME];
	char ismobshare[MAXNAME];
	char isreturn[MAXNAME];
	char click_bool[MAXNAME];
	char booking_bool[MAXNAME];
	char starttime_click[MAXNAME];
	char starttime_book[MAXNAME];
	char os[MAXNAME];
	char osversion[MAXNAME];
	FILE *fp;

	int iarg;
	char fname[MAXNAME];
	bool with_header = true;
	while ((iarg = getopt(argc, argv, "i:xh")) != -1) {
		switch (iarg) {
			case 'i':
				strcpy(fname, optarg);
				break;
			case 'x':
				with_header = false;
				break;
			case 'h':
				printf("usage: run [-i I] [-x]\n");
				printf("optional arguments:\n");
				printf("	-h show this help message and exit\n");
				printf("	-i specify input file\n");
				printf("	-x no header in input file\n");
				exit(0);
			default:
				printf("unhandled option, abort...\n");
				exit(1);
		}
	}
	fp = fopen(fname, "r");

	// header
	if (with_header)
		fgets(line, MAXLINE, fp);
	while (fgets(line, MAXLINE, fp) != NULL) {
		size_t query = 0;
		int num = 
		sscanf(line, "%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];\
									%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];\
									%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];\
									%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;];%[^;]\n", 
			uid,qid_new,starttime,hotelnum,hotelid,
			rank_in_qid,price_qry,star_qry,keyword_qry,cityid_qry,checkin,checkout,spec_qry,
			brand1_qry,brand2_qry,zone_qry,bookable_qry,dist_qry,fac_qry,filter_score,
			filter_quantity,com,sta,distr,metro1,metro2,hotd,sort_qry,bedtype,breakfast,
			paytype,fh_price,dist,isprom,isgift,iscoupon,ismobshare,isreturn,click_bool,
			booking_bool,starttime_click,starttime_book,os,osversion);
		if (num == 44) {
			// price
			if (strcmp(price_qry, "nolimit") && strcmp(price_qry, "不限") && strcmp(price_qry, "-999") && strcmp(price_qry, ""))
				query |= 1;
			// star
			if (strcmp(star_qry, "nolimit") && strcmp(star_qry, "-999") && strcmp(star_qry, ""))
				query |= 1 << 1;
			// keyword
            if (strcmp(keyword_qry, "nolimit") && strcmp(keyword_qry, "-999") && strcmp(keyword_qry, "") && strncmp(keyword_qry, "*&*&*&*&*", 9))
				query |= 1 << 2;
			// spec
			if (strcmp(spec_qry, "nolimit") && strcmp(spec_qry, "-999") && strcmp(spec_qry, "") && strcmp(spec_qry, "0"))
				query |= 1 << 3;
			// brand1 + brand2
			if ((strcmp(brand1_qry, "nolimit") && strcmp(brand1_qry, "-999") && strcmp(brand1_qry, "") && strcmp(brand1_qry, "0") && strcmp(brand1_qry, "酒店类型")) ||
					(strcmp(brand2_qry, "-999") && strcmp(brand2_qry, "") && strcmp(brand2_qry, "0") && strcmp(brand2_qry, "nolimit")))
				query |= 1 << 4;
			// dist
			if (strcmp(dist_qry, "nolimit") && strcmp(dist_qry, "不限") && strcmp(dist_qry, "-999") && strcmp(dist_qry, "") && 
					strcmp(dist_qry, "0") && strcmp(dist_qry, "4") && strcmp(dist_qry, "4公里内"))
				query |= 1 << 5;
			// fac
			if (strcmp(fac_qry, "nolimit") && strcmp(fac_qry, "不限") && strcmp(fac_qry, "-999") && strcmp(fac_qry, ""))
				query |= 1 << 6;
			// filter_score
			if (strcmp(filter_score, "nolimit") && strcmp(filter_score, "不限") && strcmp(filter_score, "-999") && strcmp(filter_score, ""))
				query |= 1 << 7;
			// filter_quantity
			if (strcmp(filter_quantity, "nolimit") && strcmp(filter_quantity, "不限") && strcmp(filter_quantity, "-999") && strcmp(filter_quantity, ""))
				query |= 1 << 8;
			// com
			if (strcmp(com, "nolimit") && strcmp(com, "不限") && strcmp(com, "-999") && strcmp(com, ""))
				query |= 1 << 9;
			// sta
			if (strcmp(sta, "nolimit") && strcmp(sta, "不限") && strcmp(sta, "-999") && strcmp(sta, ""))
				query |= 1 << 10;
			// district
			if (strcmp(distr, "nolimit") && strcmp(distr, "不限") && strcmp(distr, "-999") && strcmp(distr, ""))
				query |= 1 << 11;
			// metro1 +  metro2
			if ((strcmp(metro1, "nolimit") && strcmp(metro1, "不限") && strcmp(metro1, "-999") && strcmp(metro1, ""))||
					(strcmp(metro2, "不限") && strcmp(metro2, "-999") && strcmp(metro2, "") && strcmp(metro2, "nolimit")))
				query |= 1 << 12;
			// hotd
			if (strcmp(hotd, "nolimit") && strcmp(hotd, "不限") && strcmp(hotd, "-999") && strcmp(hotd, ""))
				query |= 1 << 13;
			// sort_qry
			if (strcmp(sort_qry, "nolimit") && strcmp(sort_qry, "不限") && strcmp(sort_qry, "-999") && strcmp(sort_qry, "") && strcmp(sort_qry, "0") && strcmp(sort_qry, "6"))
				query |= 1 << 14;
			// bedtype
			if (strcmp(bedtype, "nolimit") && strcmp(bedtype, "不限") && strcmp(bedtype, "-999") && strcmp(bedtype, ""))
				query |= 1 << 15;
			// breakfast
			if (strcmp(breakfast, "nolimit") && strcmp(breakfast, "不限") && strcmp(breakfast, "-999") && strcmp(breakfast, ""))
				query |= 1 << 16;
			// paytype
			if (strcmp(paytype, "nolimit") && strcmp(paytype, "不限") && strcmp(paytype, "-999") && strcmp(paytype, ""))
				query |= 1 << 17;

			char tmp[NCHOICE];
			memset(tmp, '0', sizeof(tmp));
			binary_fmt(query, tmp);
			
			char purchase;
			if (strcmp(booking_bool, "0"))
				purchase = 'Y';
			else purchase = 'N';
			
			char click;
			if (strcmp(click_bool, "0"))
				click = 'T';
			else click = 'F';

			// remove the blank after ¥ signs in price_qry
			size_t length = strlen(price_qry);
			for (int i = 0; i < length; ++i)
				if (price_qry[i] == ' ')
					price_qry[i] = '$';
			printf("%-36s %-24s %s %c %c %s %s %s %s %s\n", uid, starttime, tmp, click, purchase, 
				qid_new, checkin, checkout, price_qry, fh_price);
			//printf("usr:%s fp:%s \n pr:%s star:%s key:%s spec:%s brd1:%s brd2:%s dist:%s fac:%s filters:%s flt_q:%s com:%s sta:%s distr:%s m1:%s m2:%s hotd:%s sort:%s bed:%s brkf:%s pay:%s\n", 
			//        uid, binary_fmt(query, tmp), price_qry, star_qry, keyword_qry, spec_qry, brand1_qry, brand2_qry,
			//        dist_qry, fac_qry, filter_score, filter_quantity, com, sta, distr, metro1, metro2, hotd, 
			//        sort_qry, bedtype, breakfast, paytype);
		} else {
			printf("Error\n");
			printf("Line %s", line);
			exit(1);
		}
		//printf("%s\t%s\t%s\t%s\n", uid, qid, starttime, hotelnum);
		/*printf("%s %s %s %s %s %s %s %s %s %s %s \
						%s %s %s %s %s %s %s %s %s %s %s \
						%s %s %s %s %s %s %s %s %s %s %s \
						%s %s %s %s %s %s %s %s %s %s %s \n",
			uid,qid_new,starttime,hotelnum,hotelid,
			rank_in_qid,price_qry,star_qry,keyword_qry,cityid_qry,checkin,checkout,spec_qry,
			brand1_qry,brand2_qry,zone_qry,bookable_qry,dist_qry,fac_qry,filter_score,
			filter_quantity,com,sta,distr,metro1,metro2,hotd,sort_qry,bedtype,breakfast,
			paytype,fh_price,dist,isprom,isgift,iscoupon,ismobshare,isreturn,click_bool,
			booking_bool,starttime_click,starttime_book,os,osversion);*/
	}
	fclose(fp);
}

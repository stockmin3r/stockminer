/**********************************************
 * filename: backpage/monster/monster.js
 * License:  Public Domain
 *********************************************/
var openedRows_SB150=new Set(),openedRows_SB1K=new Set();
var table_refresh_SB150=0,table_refresh_SB1K=0,table_refresh_ESP=0,table_refresh_MSR=0,multi_open_SB150=0,multi_open_SB1K=0;
var childRows_SB150=null,childRows_SB1K=null;
var max_esp=0,min_esp=0,tick_esp=null,msr_price_min=0,msr_price_max=5000;
var msr_delta_min=-99.0,msr_delta_max=8000.0,msr_peak_min=-99.0,msr_peak_max=8000.0,msr_peak_peak=0,msr_peak_bear=0;
var msr_one_min=0,msr_one_max=16,msr_two_min=0,msr_two_max=16,msr_delta_pos=0,msr_delta_neg=0;
var msr_price_filter=0,msr_delta_filter=0,msr_peak_filter=0,msr_price_filter=0,msr_one_filter=0,msr_two_filter=0,msr_ytd_filter=0,msr_ytd19_filter=0,msr_ytd18_filter=0;
var msr_esp4_filter=0,msr_esp1_filter=0,msr_filter=0;
var msr_ytd_min=-99.0,msr_ytd_max=5000.0,msr_ytd19_min=-99.0,msr_ytd_max=5000.0,msr_ytd18_min=-99.0,msr_ytd18_max=5000.0;
var msr_esp_a1=0,msr_esp_a4=0,msr_esp_master=1;
var five_1q_filter=0,five_2q_filter=0,ten_1q_filter=0,ten_2q_filter=0;
var five_stg_anyday_20=0,five_stg_a1_20=0,five_stg_a4_20=0,five_stg_anyday_19=0,five_stg_a1_19=0,five_stg_a4_19=0,five_stg_anyday_18=0,five_stg_a1_18=0,five_stg_a4_18=0;
var ten_stg_anyday_20=0,ten_stg_a1_20=0,ten_stg_a4_20=0,ten_stg_anyday_19=0,ten_stg_a1_19=0,ten_stg_a4_19=0,ten_stg_anyday_18=0,ten_stg_a1_18=0,ten_stg_a4_18=0;
var q1_5min20=0,q1_5min19=0,q1_5min18=0,q1_5=0,q2_5min20=0,q2_5min19=0,q2_5min18=0,q2_5=0;
var q1_10min20=0,q1_10min19=0,q1_10min18,q1_10=0,q2_10min20=0,q2_10min19=0,q2_10min18,q2_10=0;
var avg_5min20=0,avg_5min19=0,avg_5min18=0,avg_5=0,avg_10min20=0,avg_10min19=0,avg_10min18=0,avg_10=0;
var five_2020=0,five_2019=0,five_2018=0,ten_2020=0,ten_2019=0,ten_2018=0;
var msr_table_update=0,msr_font_size=11,msr_ticker=null;

function init_monster_tables()
{
	MSR_MENU  = document.getElementById("MSR_MENU");

	T['SB150'] = SB150_table = $('#SB150').DataTable({
        "columns": [{"className":'control',"orderable":false,"data":null,"defaultContent": ''},
            { "data": "ticker"       },
			{ "data": "rank"         },
			{ "data": "price"        },
			{ "data": "chgpc"        },
			{ "data": "1yrpkpc"      },
			{ "data": "1yrpk"        },
			{ "data": "one"          },
			{ "data": "two"          },
			{ "data": "a1q1_2020"    },
			{ "data": "a1q2_2020"    },
			{ "data": "a1q1_2019"    },
			{ "data": "a1q2_2019"    },
			{ "data": "a1q1_2018"    },
			{ "data": "a1q2_2018"    },
			{ "data": "a1q1_2017"    },
			{ "data": "a1q2_2017"    },
			{ "data": "total1"       },
			{ "data": "total2"       },
			{ "data": "total3"       },
			{ "data": "total4"       },
			{ "data": "a4q1_2020"    },
			{ "data": "a4q2_2020"    },
			{ "data": "a4q1_2019"    },
			{ "data": "a4q2_2019"    },
			{ "data": "a4q1_2018"    },
			{ "data": "a4q2_2018"    },
			{ "data": "a4q1_2017"    },
			{ "data": "a4q2_2017"    },
			{ "data": "total5"       },
			{ "data": "total6"       },
			{ "data": "total7"       },
			{ "data": "total8"       },
  ],
  "columnDefs":[{"targets":33,"visible":false}],
	"createdRow":function(row,data,index){SB_createRow(row,data,index)},
	"bPaginate":false,"order": [[2, "asc" ]],"paging":false,"bDestroy":false,"info":false,"searching":false});

	T['SB1K'] = SB1K_table = $('#SB1K').DataTable({
        "columns": [{"className":'control',"orderable":false,"data":null,"defaultContent": ''},
            { "data": "ticker"       },
			{ "data": "rank"         },
            { "data": "price"        },
            { "data": "chgpc"        },
            { "data": "1yrpkpc"      },
            { "data": "1yrpk"        },
			{ "data": "one"          },
			{ "data": "two"          },
			{ "data": "a1q1_2020"    },
			{ "data": "a1q2_2020"    },
			{ "data": "a1q1_2019"    },
			{ "data": "a1q2_2019"    },
			{ "data": "a1q1_2018"    },
			{ "data": "a1q2_2018"    },
			{ "data": "a1q1_2017"    },
			{ "data": "a1q2_2017"    },
			{ "data": "total1"       },
			{ "data": "total2"       },
			{ "data": "total3"       },
			{ "data": "total4"       },
			{ "data": "a4q1_2020"    },
			{ "data": "a4q2_2020"    },
			{ "data": "a4q1_2019"    },
			{ "data": "a4q2_2019"    },
			{ "data": "a4q1_2018"    },
			{ "data": "a4q2_2018"    },
			{ "data": "a4q1_2017"    },
			{ "data": "a4q2_2017"    },
			{ "data": "total5"       },
			{ "data": "total6"       },
			{ "data": "total7"       },
			{ "data": "total8"       },
  ],"columnDefs":[{"targets":33,"visible":false}],
	"createdRow": function (row,data,index){SB_createRow(row,data,index)},
	"bPaginate":false,"order": [[2, "asc" ]],"paging":false,"bDestroy":false,"info":false,"searching":false});

	T['ESP'] = ESP_table = $('#ESP').DataTable({
        "columns": [
            { "data": "T"     },
            { "data": "DA"    },
            { "data": "R"     },
            { "data": "P"     },
            { "data": "D"     },
			{ "data": "CL"    },
			{ "data": "PK"    },
			{ "data": "O"     },
			{ "data": "W"     },
			{ "data": "a1q1"  },
			{ "data": "a1q2"  },
			{ "data": "a1T"   },
			{ "data": "a4q1"  },
			{ "data": "a4q2"  },
			{ "data": "a4T"   },
			{ "data": "A"     },
			{ "data": "esp"   },
			{ "data": "a1esp" },
			{ "data": "a1s"   },
			{ "data": "a4esp" },
			{ "data": "a4s"   },
			{ "data": "nda"   },
			{ "data": "next"  },
			{ "data": "1"     },
			{ "data": "2"     },
			{ "data": "3"     },
			{ "data": "4"     },
			{ "data": "5"     },
			{ "data": "6"     },
			{ "data": "7"     },
			{ "data": "8"     },
			{ "data": "9"     },
			{ "data": "a"     },
			{ "data": "b"     },
			{ "data": "c"     },
			{ "data": "d"     },
			{ "data": "e"     },
			{ "data": "f"     },
  ],
  "columnDefs": [{"targets":38,"visible":false},
          { "width": "36px", "targets":[0,5,4,16,17,18,19]},
		  { "width": "40px", "targets":[1,8,9,11,12]},
		  { "width": "50px", "targets":3},
		  { "width": "29px", "targets":[10,13,6,7,14,15]},
		  { type: "num-fmt", "targets": '_all'},
	],
	"createdRow": function (row,data,index){esp_colors(row,data,index)},
		dom: 'Qlfrtip',stateSave:true,"deferRender":true,
		"bPaginate":false,"order":[[1,"desc"]],"paging":false,"bDestroy":false,"autoWidth": false,"info":false,"searching":true
	});

	/* MONSTER 1.0 */
	T['MSR'] = MSR_table = $('#MSR').DataTable({
        "columns": [
            { "data": "T"               },
            { "data": "P"               },
            { "data": "D"               },
            { "data": "PK"              },
			{ "data": "1"               },
			{ "data": "2"               },
			{ "data": "a1esp"           },
			{ "data": "a4esp"           },
			{ "data": "YTD"             },
			{ "data": "y1"              },
			{ "data": "y2"              },
			{ "data": "y3"              },
			{ "data": "y4"              },
			{ "data": "a1q1_2020_any5"  },
			{ "data": "a1q2_2020_any5"  },
			{ "data": "a1q1_2019_any5"  },
			{ "data": "a1q2_2019_any5"  },
			{ "data": "a1q1_2018_any5"  },
			{ "data": "a1q2_2018_any5"  },
			{ "data": "anyavg2020"      },
			{ "data": "anyavg2019"      },
			{ "data": "anyavg2018"      },
			{ "data": "a1q1_2020"       },
			{ "data": "a1q2_2020"       },
			{ "data": "a1q1_2019"       },
			{ "data": "a1q2_2019"       },
			{ "data": "a1q1_2018"       },
			{ "data": "a1q2_2018"       },
			{ "data": "avg5a1_2020"     },
			{ "data": "avg5a1_2019"     },
			{ "data": "avg5a1_2018"     },
			{ "data": "T1"              },
			{ "data": "T2"              },
			{ "data": "T3"              },
			{ "data": "a4q1_2020"       },
			{ "data": "a4q2_2020"       },
			{ "data": "a4q1_2019"       },
			{ "data": "a4q2_2019"       },
			{ "data": "a4q1_2018"       },
			{ "data": "a4q2_2018"       },
			{ "data": "avg5a4_2020"     },
			{ "data": "avg5a4_2019"     },
			{ "data": "avg5a4_2018"     },
			{ "data": "T4"              },
			{ "data": "T5"              },
			{ "data": "T6"              },
			{ "data": "a1q1_2020_any10" },
			{ "data": "a1q2_2020_any10" },
			{ "data": "a1q1_2019_any10" },
			{ "data": "a1q2_2019_any10" },
			{ "data": "a1q1_2018_any10" },
			{ "data": "a1q2_2018_any10" },
			{ "data": "any10avg2020"    },
			{ "data": "any10avg2019"    },
			{ "data": "any10avg2018"    },
			{ "data": "a1q1_2020_10"    },
			{ "data": "a1q2_2020_10"    },
			{ "data": "a1q1_2019_10"    },
			{ "data": "a1q2_2019_10"    },
			{ "data": "a1q1_2018_10"    },
			{ "data": "a1q2_2018_10"    },
			{ "data": "avg10a1_2020"    },
			{ "data": "avg10a1_2019"    },
			{ "data": "avg10a1_2018"    },
			{ "data": "T7"              },
			{ "data": "T8"              },
			{ "data": "T9"              },
			{ "data": "a4q1_2020_10"    },
			{ "data": "a4q2_2020_10"    },
			{ "data": "a4q1_2019_10"    },
			{ "data": "a4q2_2019_10"    },
			{ "data": "a4q1_2018_10"    },
			{ "data": "a4q2_2018_10"    },
			{ "data": "avg10a4_2020"    },
			{ "data": "avg10a4_2019"    },
			{ "data": "avg10a4_2018"    },
			{ "data": "T10"             },
			{ "data": "T11"             },
			{ "data": "T12"             },
  ],
  "columnDefs":[{"targets":72,"visible":false},
          { "width": "45px", "targets":[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71]},
		  { type: 'natural', "targets": '_all'},],
	"createdRow": function (row,data,index){msr_colors(row,data,index)},
		"bPaginate":false,"order": [[0, "asc" ]],"deferRender":true,"paging":false,"bDestroy":false,"autoWidth":false,"info":false,"searching":true});

	/* MONSTER 2.0 */
	T['MSR2'] = MSR2_table = $('#MSR2').DataTable({
        "columns": [
            { "data": "T"},       /* 0  b */
			{ "data": "2" },      /* 1  s */
			{ "data": "any"},     /* 2  b */
			{ "data": "anyret"},  /* 3  b */
			{ "data": "anystd"},  /* 4  b */
			{ "data": "anyavg"},  /* 5  s */
			{ "data": "a1q1"},    /* 6  b */
			{ "data": "a1ret"},   /* 7  b */
			{ "data": "a1std"},   /* 8  b */
			{ "data": "avgA1q1"}, /* 9  s */
			{ "data": "a1T"},     /* 10 s */
			{ "data": "a4q1"},    /* 11 b */
			{ "data": "a4ret"},   /* 12 b */
			{ "data": "a4std"},   /* 13 b */
			{ "data": "avgA4q1"}, /* 14 s */
			{ "data": "a4T"}, /* 15 s */
			{ "data": "a4T"}, /* 15 s */
  ],"columnDefs":[{"targets":16,"visible":false},
			{ "width": "50px", "targets":[0,2,3,6,7,11,12]},
			{ "width": "42px", "targets":[1,5,9,10,14,15]},
			{ "width": "56px", "targets":[4,8,13]},
			{ type: 'natural', "targets": '_all'},],
	"createdRow": function (row,data,index){msr2_colors(row,data,index)},
		"deferRender":true,"bPaginate":false,"order": [[5,"desc"]],"paging":false,"bDestroy":false,"autoWidth": false,"info":false,"searching":true});

	/* Color Trend */
	T['CMT'] = CMT_table = $('#CMT').DataTable({
        "columns": [
			{ "data": "rank"  },
            { "data": "stock" },
            { "data": "date"  },
            { "data": "two"   },
            { "data": "high"  },
            { "data": "low"   },
			{ "data": "close" },
			{ "data": "daily" },
			{ "data": "day3"  },
			{ "data": "day5"  },
			{ "data": "day8"  },
			{ "data": "day13" },
			{ "data": "day21" },
			{ "data": "buy"   },
			{ "data": "dbuy"  },
			{ "data": "sell"  },
			{ "data": "dsell" },
  ],
  "columnDefs": [{"targets":17,"visible":false}],
	"createdRow": function(row,data,index){cmt_colors(row,data,index)},
		"bPaginate":false,"order": [[0, "asc" ]],"deferRender":true,"paging":false,"bDestroy":false,"info":false,"searching":false});

	T['QL'] = $('#QL').DataTable({
        "columns": [
			{"data":"902"},{"data":"900"},{"data":"313"},{"data":"901"},{"data":"913"},
			{"data":"218"},{"data":"219"},{"data":"220"},{"data":"221"},{"data":"222"},{"data":"223"},
			{"data":"245"},{"data":"227"},{"data":"228"},
			{"data":"229"},{"data":"230"},{"data":"231"},{"data":"232"},
			{"data":"233"},{"data":"234"},
			{"data":"235"},{"data":"236"},{"data":"237"},{"data":"238"},
  ],
  "columnDefs": [{"targets":24,"visible":false},{ type: 'natural', "targets": [0]}],
	"createdRow": function(row,data,index){tableColors(row,data,index,"#QL thead tr",1)},
		"bPaginate":false,"order": [[0, "asc" ]],"paging":false,"bDestroy":false,"info":false,"searching":false});

	T['PW'] = $('#PW').DataTable({
        "columns": [
			{"data":"902"},{"data":"900"},{"data":"915"},{"data":"313"},{"data":"901"},{"data":"903"},{"data":"913"},
			{"data":"218"},{"data":"219"},{"data":"220"},{"data":"221"},{"data":"222"},{"data":"223"},
			{"data":"229"},{"data":"230"},{"data":"231"},{"data":"232"},
			{"data":"235"},{"data":"236"},{"data":"237"},{"data":"238"},{"data":"247"},{"data":"253"}
  ],
  "columnDefs": [{"targets":23,"visible":false},{ type: 'natural', "targets": [0]}],
	"createdRow": function(row,data,index){tableColors(row,data,index,"#PW thead tr",1,1)},
		"bPaginate":false,"order": [[0, "asc" ]],"paging":false,"bDestroy":false,"info":false,"searching":false});

	T['FDT'] = $('#FDT').DataTable({
        "columns": [{"data":"Rank"},{"data":"Sym"},{"data":"ENDate"},{"data":"ENPrice"},{"data":"EXDate"},{"data":"EXPrice"},{"data":"RET"},{"data":"STAT"},{"data":"CLASS"}],
  "columnDefs": [{"targets":9,"visible":false},{ type: 'natural', "targets": [0]}],
	"createdRow": function(row,data,index){tableColors(row,data,index,"#PW thead tr",1)},
		dom: 'Qlfrtip',stateSave:true,"deferRender":true,
		"bPaginate":false,"order": [[0, "asc" ]],"paging":false,"bDestroy":false,"info":false,"searching":true});

	T['PORTS'] = $('#PORTS').DataTable({
        "columns": [
			{"data":"PID"},{"data":"Capital"},{"data":"RTD"},{"data":"APY"},{"data":"LP"},{"data":"T"},{"data":"S"}
  ],
  "columnDefs": [{"targets":4,"visible":false}],
	"createdRow": function(row,data,index){tableColors(row,data,index,"#PW thead tr",1)},
		"bPaginate":false,"order": [[0, "asc" ]],"paging":false,"bDestroy":false,"info":false,"searching":false});
/*
	T['QF'] = $('#QF').DataTable({
        "columns": [
			{"data":"YEAR"},{"data":"Q"},{"data":"S"},{"data":"E"},{"data":"T"},
			{"data":"APY1"},{"data":"APY2"},{"data":"APY3"},{"data":"RTD1"},{"data":"RTD2"},{"data":"RTD3"},
			{"data":"APY4"},{"data":"APY5"},{"data":"APY6"},{"data":"RTD4"},{"data":"RTD5"},{"data":"RTD6"}
  ],
  "columnDefs": [{"targets":17,"visible":false}],
	"createdRow": function(row,data,index){tableColors(row,data,index,"#QF thead tr",0)},
		"bPaginate":false,"order": [[0, "asc" ]],"paging":false,"bDestroy":false,"info":false,"searching":false});
*/

	T['QF'] = $('#QF').DataTable({
        "columns": [
			{"data":"T_1"},{"data":"P_1"},{"data":"63d_1"},{"data":"128d_1"},{"data":"dead_1"},
			{"data":"T_2"},{"data":"P_2"},{"data":"63d_2"},{"data":"128d_2"},{"data":"dead_2"},
			{"data":"T_3"},{"data":"P_3"},{"data":"63d_3"},{"data":"128d_3"},{"data":"dead_3"},
			{"data":"T_4"},{"data":"P_4"},{"data":"63d_4"},{"data":"128d_4"},{"data":"dead_4"},
			{"data":"T_5"},{"data":"P_5"},{"data":"63d_5"},{"data":"128d_5"},{"data":"dead_5"}
  ],
  "columnDefs": [{"targets":25,"visible":false}],
	"createdRow": function(row,data,index){safetab_colors(row,data,index)},
		"bPaginate":false,"paging":false,"bDestroy":false,"info":false,"ordering":false,"searching":false});

	// monster table Menu Search
	$.fn.dataTable.ext.search.push(function(settings,data,dataIndex){
/*		if (settings.nTable.id==="FDT") {
			if (data[8] != FDT[fdt_select])
				return false;
			return true;
		} else */
		if (settings.nTable.id==="MSR") {
			if (msr_esp_a1) {
				if ((data[6] === "") || parseFloat(data[19]) < 70)
					return false;
			} else if (msr_esp_a4)
				if ((data[7] === "") || parseFloat(data[31]) < 70)
					return false;

			if (msr_price_filter) {
				var price = parseFloat(data[1]);
				if (price < msr_price_min || price > msr_price_max)
					return false;
			}

			if (msr_one_filter) {
				var one = parseInt(data[4]);
				if (one < msr_one_min || one > msr_one_max)
					return false;
			}

			if (msr_two_filter) {
				var two = parseInt(data[5]);
				if (two < msr_two_min || two > msr_two_max)
					return false;
			}

			if (msr_delta_filter) {
				var delta = parseFloat(data[2]);
				if (delta < msr_delta_min || delta > msr_delta_max)
					return false;
				if (msr_delta_neg && delta >= 0)
					return false;
				if (msr_delta_pos && delta <= 0)
					return false;
			}

			if (msr_ytd_filter) {
				var ytd = parseFloat(data[8]);
				if (ytd < msr_ytd_min)
					return false;
				if (ytd > msr_ytd_max)
					return false;
			}

			if (msr_peak_filter) {
				if (msr_peak_peak && data[3] === "peak")
					return false;
				var peak = parseFloat(data[3]);
				if (msr_peak_bear && peak > -20)
					return;
				if (peak < msr_peak_min || peak > msr_peak_max)
					return false;
			}
			if (five_stg_a1_20) {
				if (q1_5min20) {
					var q1 = parseFloat(data[19]);
					if (q1 < q1_5min20)
						return false;
				} else if (q2_5min20) {
					var q2 = parseFloat(data[20]);
					if (q2 < q2_5min20)
						return false;
				} else if (avg_5min20) {
					var avg = parseFloat(data[25]);
					if (avg > avg_5min20)
						return;
				}
			} else if (five_stg_a4_20) {
				if (q1_5min20) {
					var q1 = parseFloat(data[19]);
					if (q1 < q1_5min20)
						return false;
				} else if (q2_5min20) {
					var q2 = parseFloat(data[20]);
					if (q2 < q2_5min20)
						return false;
				} else if (avg_5min20) {
					var avg = parseFloat(data[25]);
					if (avg > avg_5min20)
						return;
				}
			}

			if (five_stg_a1_19) {
				if (q1_5min19) {
					var q1 = parseFloat(data[19]);
					if (q1 < q1_5min19)
						return false;
				} else if (q2_5min19) {
					var q2 = parseFloat(data[20]);
					if (q2 < q2_5min19)
						return false;
				} else if (avg_5min19) {
					var avg = parseFloat(data[25]);
					if (avg > avg_5min19)
						return;
				}
			} else if (five_stg_a4_19) {
				if (q1_5min19) {
					var q1 = parseFloat(data[19]);
					if (q1 < q1_5min19)
						return false;
				} else if (q2_5min19) {
					var q2 = parseFloat(data[20]);
					if (q2 < q2_5min19)
						return false;
				} else if (avg_5min19) {
					var avg = parseFloat(data[25]);
					if (avg > avg_5min19)
						return;
				}
			}

			if (five_stg_a1_18) {
				if (q1_5min18) {
					var q1 = parseFloat(data[19]);
					if (q1 < q1_5min18)
						return false;
				} else if (q2_5min18) {
					var q2 = parseFloat(data[20]);
					if (q2 < q2_5min18)
						return false;
				} else if (avg_5min18) {
					var avg = parseFloat(data[25]);
					if (avg > avg_5min18)
						return;
				}
			} else if (five_stg_a4_18) {
				if (q1_5min18) {
					var q1 = parseFloat(data[19]);
					if (q1 < q1_5min18)
						return false;
				} else if (q2_5min18) {
					var q2 = parseFloat(data[20]);
					if (q2 < q2_5min18)
						return false;
				} else if (avg_5min18) {
					var avg = parseFloat(data[25]);
					if (avg > avg_5min18)
						return;
				}
			}

			if (ten_stg_a1_20) {
				if (q1_10min20) {
					var q1 = parseFloat(data[19]);
					if (q1 < q1_10min20)
						return false;
				} else if (q2_10min20) {
					var q2 = parseFloat(data[20]);
					if (q2 < q2_10min20)
						return false;
				} else if (avg_10min20) {
					var avg = parseFloat(data[25]);
					if (avg > avg_10min20)
						return;
				}
			} else if (ten_stg_a4_20) {
				if (q1_5min20) {
					var q1 = parseFloat(data[19]);
					if (q1 < q1_10min20)
						return false;
				} else if (q2_10min20) {
					var q2 = parseFloat(data[20]);
					if (q2 < q2_10min20)
						return false;
				} else if (avg_10min20) {
					var avg = parseFloat(data[25]);
					if (avg > avg_10min20)
						return;
				}
			}
			if (ten_stg_a1_19) {
				if (q1_10min19) {
					var q1 = parseFloat(data[19]);
					if (q1 < q1_10min19)
						return false;
				} else if (q2_10min19) {
					var q2 = parseFloat(data[20]);
					if (q2 < q2_10min19)
						return false;
				} else if (avg_10min19) {
					var avg = parseFloat(data[25]);
					if (avg > avg_10min19)
						return;
				}
			} else if (ten_stg_a4_19) {
				if (q1_10min19) {
					var q1 = parseFloat(data[19]);
					if (q1 < q1_10min19)
						return false;
				} else if (q2_10min19) {
					var q2 = parseFloat(data[20]);
					if (q2 < q2_10min19)
						return false;
				} else if (avg_10min19) {
					var avg = parseFloat(data[25]);
					if (avg > avg_10min19)
						return;
				}
			}
			if (ten_stg_a1_18) {
				if (q1_10min18) {
					var q1 = parseFloat(data[19]);
					if (q1 < q1_10min18)
						return false;
				} else if (q2_10min18) {
					var q2 = parseFloat(data[20]);
					if (q2 < q2_10min18)
						return false;
				} else if (avg_10min18) {
					var avg = parseFloat(data[25]);
					if (avg > avg_10min18)
						return;
				}
			} else if (ten_stg_a4_18) {
				if (q1_10min18) {
					var q1 = parseFloat(data[19]);
					if (q1 < q1_10min18)
						return false;
				} else if (q2_10min18) {
					var q2 = parseFloat(data[20]);
					if (q2 < q2_10min18)
						return false;
				} else if (avg_10min18) {
					var avg = parseFloat(data[25]);
					if (avg > avg_10min18)
						return;
				}
			}
			return true;
		} else if (settings.nTable.id==="ESP") {
			if (max_esp && parseInt(data[1].replace("/", "1")) > max_esp)
				return false;
			if (min_esp && parseInt(data[1].replace("/", "1")) < min_esp)
				return false;
			if (tick_esp && tick_esp!=data[0])
				return false;
		}
		return true;
	});

	/* Monster Menu */
	$.contextMenu({
            selector: '.context-menu-one',
            items: {
                "menu":{name: "Menu",callback:function($element, key, item){
					MSR_MENU.style.display="block";
					$("#monster_menu").modal('show');
					$("#monster_menu").draggable();
				}},
	            "reset": {"name": "Reset Filters", callback:function($element, key, item){msr_reset();return true}},
                "cut":   {name: "Hide Column"},
                "sep1": "---------",
				"xls":{name:"Full XLS", selector:'.context-menu-one td', callback:function($element,key,item){
					if (msr_ticker == null)
						return false;
					var win = window.open('https://www.stockminer.world/XPAGE/' + msr_ticker);
					win.focus();
				}},
				"hyb":{name:"Hybird",selector:'.context-menu-one td', callback:function($element,key,item){
					if (msr_ticker == null)
						return false;
					hybrid_load();
				}},
				"action":{name:"ScatterPlot",callback:function($element,key,item){
					document.getElementById("cscatter").style.display="block";
					$("#scatbox").modal('show');
					scatter_chart(msr_ticker, "sbx");
					$("#scatbox").on("hidden.bs.modal", function(){
						document.getElementById("sbx").innerHTML='';
					});
					return true;
				}},
				"candle":{name:"Candlestick Chart",callback:function($element,key,item){

				}},
				"line":{name:"Line Chart",callback:function($element,key,item){

				}},
				"esp":{name:"Master ESP",callback:function($element,key,item){
					msr_esp_a1 = 1;
					msr_esp_a4 = 1;
					MSR_table.draw();
				}},
				"a1esp":{name:"Action 1 ESP",callback:function($element,key,item){esp_a1()}},
				"a4esp":{name:"Action 4 ESP",callback:function($element,key,item){esp_a4()}},
				"sep2": "---------",
				"inc":{name:"Increase Table Size", callback:function($element,key,item){
					$("#MSR thead th").css("font-size", ++msr_font_size);
					$("#MSR tbody td").css("font-size", ++msr_font_size);
				}},
				"dec":{name:"Decrease Table Size", callback:function($element,key,item){
					$("#MSR thead th").css("font-size", --msr_font_size);
					$("#MSR tbody td").css("font-size", --msr_font_size);
                }},
			}
        });

    $('#SB150').on('click', 'td.control', function () {
        var tr  = $(this).closest('tr');
        var row = SB150_table.row(tr);
        if (row.child.isShown()) {
            row.child.hide();
            tr.removeClass('shown');
            openedRows_SB150.delete(row.index());
		if (multi_open_SB150)
			multi_open_SB150-=1;
		else
			table_refresh_SB150=1;
        } else {
		d = row.data();
		sb150_xls(d, row, tr);
        }
    });
    $('#SB1K').on('click', 'td.control', function () {
        var tr  = $(this).closest('tr');
        var row = SB1K_table.row(tr);
        if (row.child.isShown()) {
            row.child.hide();
            tr.removeClass('shown');
            openedRows_SB1K.delete(row.index());
		if (multi_open_SB1K)
			multi_open_SB1K-=1;
		else
			table_refresh_SB1K=1;
        } else {
		d = row.data();
		sb1K_xls(d, row, tr);
        }
    });
	SB150_table.on('draw', function () {
		if (childRows_SB150) {
			childRows_SB150.every(function(rowIdx, tableLoop, rowLoop) {
				d = this.data();
 				this.child($(sb150_xls(d))).show();
				this.nodes().to$().addClass('shown');
			});
			childRows_SB150 = null;
		}
	});
	SB1K_table.on('draw', function () {
		if (childRows_SB1K) {
			childRows_SB1K.every(function(rowIdx, tableLoop, rowLoop) {
				d = this.data();
 				this.child($(sb1K_xls(d))).show();
				this.nodes().to$().addClass('shown');
			});
			childRows_SBK = null;
		}
	});
	setInterval(function () {
	if (table_refresh_SB150) {
	    SB150_table.ajax.reload(function () {
	    openedRows_SB150.forEach(row => {
	      SB150_table.row(row).child(sb150_xls(SB150_table.row(row).data())).show();
	      SB150_table.row(row).nodes().to$().addClass('shown');
	    });
	    }, false);
	}
  }, 3000);
	$("#selectbox > ul > li").click(function()   {$(this).removeClass('boxtick');$(this).addClass('boxtick')});
}

function hybrid(){
	var ticker = document.getElementById("HQ").value;
	$.ajax({url:"/HYB/"+ticker, async:"true", complete: function(result){
		var tables = result.responseText.split("@");
		var mini = tables[0];
		var esp  = tables[1];
		var msr  = tables[2];
		var msr2 = tables[3];
		var mini_table = document.getElementById("HYB_MINI");
		var esp_table  = document.getElementById("HYB_ESP");
		var msr_table  = document.getElementById("HYB_MSR");
		var msr2_table = document.getElementById("HYB_MSR2");
		$(mini_table).find('tbody').append(mini);
		$(esp_table).find('tbody').append(esp);
		$(msr_table).find('tbody').append(msr);
		$(msr2_table).find('tbody').append(msr2);
	}});
}

function minesp(){
	min_esp = document.getElementById("minesp").value.replace("/", "1");
	min_esp_set=1;
	if (max_esp_set==1)
		ESP_table.draw();
}
function maxesp(){
	max_esp = document.getElementById("maxesp").value.replace("/", "1");
	max_esp_set=1;
	if (min_esp_set)
		ESP_table.draw();
}
function tickesp(){
	tick_esp = document.getElementById("tickesp").value;
	ESP_table.draw();
}

function price_mini()
{
	var mini = document.getElementById("price_mini").value;
	msr_price_min = mini;
	msr_table_update = 1;
	msr_price_filter=1;
	msr_filter=1;
}
function price_maxi()
{
	var maxi = document.getElementById("price_maxi").value;
	msr_price_max = maxi;
	msr_table_update = 1;
	msr_price_filter=1;
	msr_filter=1;
}

function delta_mini()
{
	var mini = document.getElementById("delta_mini").value;
	msr_delta_min = mini;
	msr_table_update = 1;
	msr_delta_filter=1;
	msr_filter=1;
}
function delta_maxi()
{
	var maxi = document.getElementById("delta_maxi").value;
	msr_delta_max = maxi;
	msr_table_update = 1;
	msr_delta_filter=1;
	msr_filter=1;
}
function ytd_mini()
{
	var mini = document.getElementById("ytd_mini").value;
	msr_ytd_min = mini;
	msr_table_update = 1;
	msr_ytd_filter=1;
	msr_filter=1;
}
function ytd_maxi()
{
	var maxi = document.getElementById("ytd_maxi").value;
	msr_ytd_max = maxi;
	msr_table_update = 1;
	msr_ytd_filter=1;
	msr_filter=1;
}
function ytd19_mini()
{
	var mini = document.getElementById("ytd19_mini").value;
	msr_ytd19_min = mini;
	msr_table_update = 1;
	msr_filter=1;
}
function ytd19_maxi()
{
	var maxi = document.getElementById("ytd19_maxi").value;
	msr_ytd19_max = maxi;
	msr_table_update = 1;
	msr_ytd19_filter=1;
	msr_filter=1;
}
function ytd18_mini()
{
	var mini = document.getElementById("ytd18_mini").value;
	msr_ytd18_min = mini;
	msr_ytd18_filter=1;
	msr_table_update = 1;
	msr_filter=1;
}
function ytd18_maxi()
{
	var maxi = document.getElementById("ytd18_maxi").value;
	msr_ytd18_max = maxi;
	msr_ytd18_filter=1;
	msr_table_update = 1;
	msr_filter=1;
}
function delta_pos()
{
	msr_delta_pos = 1;
	$("#delta_neg").prop("checked", false);
	msr_delta_neg = 0;
	msr_delta_filter=1;
	msr_table_update = 1;
	msr_filter=1;
}
function delta_neg()
{
	msr_delta_neg = 1;
	$("#delta_pos").prop("checked", false);
	msr_delta_pos = 0;
	msr_delta_filter=1;
	msr_table_update = 1;
	msr_filter=1;
}
function peak_peak()
{
	msr_peak_bear = 0;
	msr_peak_peak = 1;
	msr_peak_filter=1;
	$("#peak_bear").prop("checked", false);
	msr_table_update = 1;
	msr_filter=1;
}
function peak_bear()
{
	msr_peak_bear = 1;
	msr_peak_peak = 0;
	msr_peak_filter=1;
	$("#peak_peak").prop("checked", false);
	msr_table_update = 1;
	msr_filter=1;
}
function one_mini()
{
	var mini = document.getElementById("one_mini").value;
	msr_one_min = mini;
	msr_one_filter=1;
	msr_table_update = 1;
	msr_filter=1;
}
function one_maxi()
{
	var maxi = document.getElementById("one_maxi").value;
	msr_one_max = maxi;
	msr_one_filter=1;
	msr_table_update = 1;
	msr_filter=1;
}
function two_mini()
{
	var mini = document.getElementById("two_mini").value;
	msr_two_min = mini;
	msr_two_filter=1;
	msr_table_update = 1;
	msr_filter=1;
}
function two_maxi()
{
	var maxi = document.getElementById("two_maxi").value;
	msr_two_max = maxi;
	msr_two_filter=1;
	msr_table_update = 1;
	msr_filter=1;
}
function esp_a1()
{
	msr_esp_a1 = 1;
	msr_esp_a4 = 0;
	msr_esp_filter=1;
	msr_table_update = 1;
	msr_filter=1;
}
function esp_a4()
{
	msr_esp_a4 = 1;
	msr_esp_a1 = 0;
	msr_esp_filter=1;
	msr_table_update = 1;
	msr_filter=1;
}
function esp_master()
{
	msr_esp_master = 1;
	msr_esp_a4 = 1;
	msr_esp_a1 = 1;
	msr_table_update = 1;
	msr_filter=1;
}
function msr_reset()
{
	msr_esp_a1=0;
	msr_esp_a4=0;
	msr_filter=0;
	msr_esp_master=1;
	msr_esp_filter=0;
	msr_one_filter=0;
	msr_two_filter=0;
	msr_delta_filter=0;
	msr_peak_filter=0;
	msr_price_filter=0;
	msr_ytd_filter=0;
	msr_ytd19_filter=0;
	msr_ytd18_filter=0;
	five_stg_anyday_20=0;
	five_stg_a1_20=0;
	five_stg_a4_20=0;
	five_stg_anyday_19=0;
	five_stg_a1_19=0;
	five_stg_a4_19=0;
	five_stg_anyday_18=0;
	five_stg_a1_18=0;
	five_stg_a4_18=0;
	ten_stg_anyday_20=0;
	ten_stg_a1_20=0;
	ten_stg_a4_20=0;
	ten_stg_anyday_19=0;
	ten_stg_a1_19=0;
	ten_stg_a4_19=0;
	ten_stg_anyday_18=0;
	ten_stg_a1_18=0;
	ten_stg_a4_18=0;
	q1_5min20=0;
	q2_5min20=0;
	avg_5min20=0;
	avg_5=0;
	avg_10=0;
	q1_5min19=0;
	q2_5min19=0;
	avg_5min19=0;
	q1_5min18=0;
	q2_5min18=0;
	avg_5min18=0;
	q1_10min20=0;
	q2_10min20=0;
	avg_10min20=0;
	q1_10min19=0;
	q2_10min19=0;
	avg_10min19=0;
	q1_10min18=0;
	q2_10min18=0;
	avg_10min18=0;
	five_2020=0;
	five_2019=0;
	five_2018=0;
	ten_2020=0;
	ten_2019=0;
	ten_2018=0;
	msr_ytd_min=0;
	msr_ytd_max=0;
	msr_ytd19_min=0;
	msr_ytd19_max=0;
	msr_ytd18_min=0;
	msr_ytd18_max=0;
	msr_price_min=0;
	msr_price_max=5000;
	msr_delta_min=-99;
	msr_delta_max=8000;
	msr_peak_min=-99;
	msr_peak_max=8000;
	msr_peak_peak=0;
	msr_peak_bear=0;
	msr_delta_neg=0;
	msr_delta_pos=0;
	msr_one_min=0;
	msr_one_max=16;
	msr_two_min=0;
	msr_two_max=16;
	MSR_table.draw();
}

function msr_apply()
{
	MSR_table.draw();
	msr_table_update=0;
}
function five_input_1q()
{
	var q1 = document.getElementById("menu_5q1").value;
	if (document.getElementById("year_2021").checked == true) {
		q1_5min20 = q1;
		msr_table_update=1;
	} else if (document.getElementById("year_2020").checked == true) {
		q1_5min19 = q1;
		msr_table_update=1;
	} else if (document.getElementById("year_2019").checked == true) {
		q1_5min18 = q1;
		msr_table_update=1;
	} else
		q1_5 = q1;
}
function five_input_2q()
{
	var q2 = document.getElementById("menu_5q2").value;
	if (document.getElementById("year_2021").checked == true) {
		msr_table_update=1;
		q2_5min20 = q2;
	} else if (document.getElementById("year_2020").checked==true) {
		q2_5min19 = q2;
		msr_table_update=1;
	} else if (document.getElementById("year_2019").checked==true) {
		q2_5min18 = q2;
		msr_table_update=1;
	} else
		q2_5 = q2;
}
function five_input_avg()
{
	var avg = document.getElementById("menu_5avg").value;
	if (document.getElementById("year_2021").checked==true) {
		msr_table_update=1;
		avg_5min20 = avg;
	} else if (document.getElementById("year_2020").checked==true) {
		avg_5min19 = avg;
		msr_table_update=1;
	} else if (document.getElementById("year_2019").checked==true) {
		avg_5min18 = avg;
		msr_table_update=1;
	} else
		avg_5 = avg;
}

function ten_input_1q()
{
	var q1 = document.getElementById("menu_10q1").value;
	if (document.getElementById("year10_2021").checked == true) {
		q1_10min20 = q1;
		msr_table_update=1;
	} else if (document.getElementById("year10_2020").checked == true) {
		q1_10min19 = q1;
		msr_table_update=1;
	} else if (document.getElementById("year10_2019").checked == true) {
		q1_10min18 = q1;
		msr_table_update=1;
	} else
		q1_10 = q1;
}
function ten_input_2q()
{
	var q2 = document.getElementById("menu_10q2").value;
	if (document.getElementById("year10_2021").checked == true) {
		msr_table_update=1;
		q2_10min20 = q2;
	} else if (document.getElementById("year10_2020").checked==true) {
		q2_10min19 = q2;
		msr_table_update=1;
	} else if (document.getElementById("year10_2019").checked==true) {
		q2_10min18 = q2;
		msr_table_update=1;
	} else
		q2_10 = q2;
}

function ten_input_avg()
{
	var avg = document.getElementById("menu_10avg").value;
	if (document.getElementById("year10_2021").checked==true) {
		msr_table_update=1;
		avg_10min20 = avg;
	} else if (document.getElementById("year10_2020").checked==true) {
		avg_10min19 = avg;
		msr_table_update=1;
	} else if (document.getElementById("year10_2019").checked==true) {
		avg_10min18 = avg;
		msr_table_update=1;
	} else
		avg_10 = avg;
}

function five_check_1q()
{
}
function five_check_2q()
{
}
function five_check_avg()
{
}
function ten_check_1q()
{
}
function ten_check_2q()
{
}
function ten_check_avg()
{
}

function strategy_5anyday()
{
	if (document.getElementById("year_2021").checked) {
		five_stg_anyday_20=1;
		five_stg_a1_20=0;
		five_stg_a4_20=0;
		if (q1_5min20||q2_5min20||avg_5min20)
			msr_table_update=1;
	} else if (document.getElementById("year_2020").checked) {
		five_stg_anyday_19=1;
		five_stg_a1_19=0;
		five_stg_a4_19=0;
		if (q1_5min19||q2_5min19||avg_5min19)
			msr_table_update=1;
	} else if (document.getElementById("year_2019").checked) {
		five_stg_anyday_18=1;
		five_stg_a1_18=0;
		five_stg_a4_18=0;
		if (q1_5min18||q2_5min18||avg_5min18)
			msr_table_update=1;
	}
	document.getElementById("check_stg5_a1").checked = false;
	document.getElementById("check_stg5_a4").checked = false;
	if (q1_5min20||q2_5min20||avg_5min20)
		msr_table_update=1;
}
function strategy_5a1()
{
	if (document.getElementById("year_2021").checked) {
		five_stg_anyday_20=0;
		five_stg_a1_20=1;
		five_stg_a4_20=0;
		if (q1_5min20||q2_5min20||avg_5min20)
			msr_table_update=1;
	} else if (document.getElementById("year_2020").checked) {
		five_stg_anyday_19=0;
		five_stg_a1_19=1;
		five_stg_a4_19=0;
		if (q1_5min19||q2_5min19||avg_5min19)
			msr_table_update=1;
	} else if (document.getElementById("year_2019").checked) {
		five_stg_anyday_18=0;
		five_stg_a1_18=1;
		five_stg_a4_18=0;
		if (q1_5min18||q2_5min18||avg_5min18)
			msr_table_update=1;
	}
	document.getElementById("check_stg5_anyday").checked = false;
	document.getElementById("check_stg5_a4").checked = false;
}
function strategy_5a4()
{
	if (document.getElementById("year_2021").checked) {
		five_stg_anyday_20=0;
		five_stg_a1_20=0;
		five_stg_a4_20=1;
		if (q1_5min20||q2_5min20||avg_5min20)
			msr_table_update=1;
	} else if (document.getElementById("year_2020").checked) {
		ten_stg_anyday_19=0;
		ten_stg_a1_19=0;
		ten_stg_a4_19=1;
		if (q1_5min19||q2_5min19||avg_5min19)
			msr_table_update=1;
	} else if (document.getElementById("year_2019").checked) {
		ten_stg_anyday_18=0;
		ten_stg_a1_18=0;
		ten_stg_a4_18=1;
		if (q1_5min18||q2_5min18||avg_5min18)
			msr_table_update=1;
	}
	document.getElementById("check_stg5_a1").checked = false;
	document.getElementById("check_stg5_anyday").checked = false;
}

function strategy_10anyday()
{
	if (document.getElementById("year10_2021").checked) {
		ten_stg_anyday_20=1;
		ten_stg_a1_20=0;
		ten_stg_a4_20=0;
		if (q1_10min20||q2_10min20||avg_10min20)
			msr_table_update=1;
	} else if (document.getElementById("year10_2020").checked) {
		ten_stg_anyday_19=1;
		ten_stg_a1_19=0;
		ten_stg_a4_19=0;
		if (q1_10min19||q2_10min19||avg_10min19)
			msr_table_update=1;
	} else if (document.getElementById("year10_2019").checked) {
		ten_stg_anyday_18=1;
		ten_stg_a1_18=0;
		ten_stg_a4_18=0;
		if (q1_10min18||q2_10min18||avg_10min18)
			msr_table_update=1;
	}
	document.getElementById("check_stg10_a1").checked = false;
	document.getElementById("check_stg10_a4").checked = false;
	if (q1_10min20||q2_10min20||avg_10min20)
		msr_table_update=1;
}
function strategy_10a1()
{
	if (document.getElementById("year10_2021").checked) {
		ten_stg_anyday_20=0;
		ten_stg_a1_20=1;
		ten_stg_a4_20=0;
		if (q1_10min20||q2_10min20||avg_10min20)
			msr_table_update=1;
	} else if (document.getElementById("year10_2020").checked) {
		ten_stg_anyday_19=0;
		ten_stg_a1_19=1;
		ten_stg_a4_19=0;
		if (q1_10min19||q2_10min19||avg_10min19)
			msr_table_update=1;
	} else if (document.getElementById("year10_2019").checked) {
		ten_stg_anyday_18=0;
		ten_stg_a1_18=1;
		ten_stg_a4_18=0;
		if (q1_10min18||q2_10min18||avg_10min18)
			msr_table_update=1;
	}
	document.getElementById("check_stg10_anyday").checked = false;
	document.getElementById("check_stg10_a4").checked = false;
}
function strategy_10a4()
{
	if (document.getElementById("year10_2021").checked) {
		ten_stg_anyday_20=0;
		ten_stg_a1_20=0;
		ten_stg_a4_20=1;
		if (q1_10min20||q2_10min20||avg_10min20)
			msr_table_update=1;
	} else if (document.getElementById("year10_2020").checked) {
		ten_stg_anyday_19=0;
		ten_stg_a1_19=0;
		ten_stg_a4_19=1;
		if (q1_10min19||q2_10min19||avg_10min19)
			msr_table_update=1;
	} else if (document.getElementById("year10_2019").checked) {
		ten_stg_anyday_18=0;
		ten_stg_a1_18=0;
		ten_stg_a4_18=1;
		if (q1_10min18||q2_10min18||avg_10min18)
			msr_table_update=1;
	}
	document.getElementById("check_stg10_a1").checked = false;
	document.getElementById("check_stg10_anyday").checked = false;
}
function year_2021()
{
	$("#year_2020").prop("checked", false);
	$("#year_2019").prop("checked", false);
	if (q1_5min20 != 0) {
		document.getElementById("menu_5q1").value = q1_5min20;
		document.getElementById("check_5q1").checked = true;
	} else {
		document.getElementById("menu_5q1").value = "";
		document.getElementById("check_5q1").checked = false;
	}
	if (q2_5min20 != 0) {
		document.getElementById("menu_5q2").value = q2_5min20;
		document.getElementById("check_5q2").checked = true;
	} else {
		document.getElementById("menu_5q2").value = "";
		document.getElementById("check_5q2").checked = false;
	}
	if (avg_5min20 != 0) {
		document.getElementById("menu_5avg").value = avg_5min20;
		document.getElementById("check_5avg").checked = true;
	} else {
		document.getElementById("menu_5avg").value = "";
		document.getElementById("check_5avg").checked = false;
	}

	if (five_stg_a1_20 != 0) {
		document.getElementById("check_stg5_a1").checked = true;
		document.getElementById("check_stg5_a4").checked = false;
		document.getElementById("check_stg5_anyday").checked = false;
	}
	else
		document.getElementById("check_stg5_a1").checked = false;

	if (five_stg_a4_20 != 0) {
		document.getElementById("check_stg5_a4").checked = true;
		document.getElementById("check_stg5_a1").checked = false;
		document.getElementById("check_stg5_anyday").checked = false;
	} else
		document.getElementById("check_stg5_a4").checked = false;

	if (five_stg_anyday_20 != 0) {
		document.getElementById("check_stg5_anyday").checked = true;
		document.getElementById("check_stg5_a4").checked = false;
		document.getElementById("check_stg5_a1").checked = false;
	} else
		document.getElementById("check_stg5_anyday").checked = false;

	if (q1_5min20||q2_5min20||avg_5min20)
		msr_table_update=1;
}
function year_2020()
{
	$("#year_2021").prop("checked", false);
	$("#year_2019").prop("checked", false);
	if (q1_5min19 != 0) {
		document.getElementById("menu_5q1").value = q1_5min19;
		document.getElementById("check_5q1").checked = true;
	} else {
		document.getElementById("menu_5q1").value = "";
		document.getElementById("check_5q1").checked = false;
	}
	if (q2_5min19 != 0) {
		document.getElementById("menu_5q2").value = q2_5min19;
		document.getElementById("check_5q2").checked = true;
	} else {
		document.getElementById("menu_5q2").value = "";
		document.getElementById("check_5q2").checked = false;
	}
	if (avg_5min19 != 0) {
		document.getElementById("menu_5avg").value = avg_5min19;
		document.getElementById("check_5avg").checked = true;
	} else {
		document.getElementById("menu_5avg").value = "";
		document.getElementById("check_5avg").checked = false;
	}


	if (five_stg_a1_19 != 0) {
		document.getElementById("check_stg5_a1").checked = true;
		document.getElementById("check_stg5_a4").checked = false;
		document.getElementById("check_stg5_anyday").checked = false;
	}
	else
		document.getElementById("check_stg5_a1").checked = false;

	if (five_stg_a4_19 != 0) {
		document.getElementById("check_stg5_a4").checked = true;
		document.getElementById("check_stg5_a1").checked = false;
		document.getElementById("check_stg5_anyday").checked = false;
	} else
		document.getElementById("check_stg5_a4").checked = false;

	if (five_stg_anyday_19 != 0) {
		document.getElementById("check_stg5_anyday").checked = true;
		document.getElementById("check_stg5_a4").checked = false;
		document.getElementById("check_stg5_a1").checked = false;
	} else
		document.getElementById("check_stg5_anyday").checked = false;


	if (q1_5min19||q2_5min19||avg_5min19)
		msr_table_update=1;
}
function year_2018()
{
	$("#year_2021").prop("checked", false);
	$("#year_2020").prop("checked", false);
	if (q1_5min18 != 0) {
		document.getElementById("menu_5q1").value = q1_5min18;
		document.getElementById("check_5q1").checked = true;
	} else {
		document.getElementById("menu_5q1").value = "";
		document.getElementById("check_5q1").checked = false;
	}
	if (q2_5min18 != 0) {
		document.getElementById("menu_5q2").value = q2_5min18;
		document.getElementById("check_5q2").checked = true;
	} else {
		document.getElementById("menu_5q2").value = "";
		document.getElementById("check_5q2").checked = false;
	}
	if (avg_5min18 != 0) {
		document.getElementById("menu_5avg").value = avg_5min18;
		document.getElementById("check_5avg").checked = true;
	} else {
		document.getElementById("menu_5avg").value = "";
		document.getElementById("check_5avg").checked = false;
	}

	if (five_stg_a1_18 != 0) {
		document.getElementById("check_stg5_a1").checked = true;
		document.getElementById("check_stg5_a4").checked = false;
		document.getElementById("check_stg5_anyday").checked = false;
	}
	else
		document.getElementById("check_stg5_a1").checked = false;

	if (five_stg_a4_18 != 0) {
		document.getElementById("check_stg5_a4").checked = true;
		document.getElementById("check_stg5_a1").checked = false;
		document.getElementById("check_stg5_anyday").checked = false;
	} else
		document.getElementById("check_stg5_a4").checked = false;

	if (five_stg_anyday_18 != 0) {
		document.getElementById("check_stg5_anyday").checked = true;
		document.getElementById("check_stg5_a4").checked = false;
		document.getElementById("check_stg5_a1").checked = false;
	} else
		document.getElementById("check_stg5_anyday").checked = false;
	if (q1_5min18||q2_5min18||avg_5min18)
		msr_table_update=1;
}

function msdiag_ticker(){$(".msmenu").css("display", "none");document.getElementById("menu_ticker").style.display="block"}
function msdiag_price()
{
	$(".msmenu").css("display", "none");
	document.getElementById("menu_price").style.display="block";
	if (msr_price_min != 0)
		document.getElementById("price_mini").value = msr_price_min;
	if (msr_price_max != 5000)
		document.getElementById("price_maxi").value = msr_price_max;
}
function msdiag_delta()
{
	$(".msmenu").css("display", "none");
	document.getElementById("menu_delta").style.display="block";
	if (msr_delta_min != -99.0)
		document.getElementById("delta_mini").value = msr_delta_min;
	if (msr_delta_max != 8000.0)
		document.getElementById("delta_maxi").value = msr_delta_max;
}
function msdiag_peak()
{
	$(".msmenu").css("display", "none");
	document.getElementById("menu_peak").style.display="block";
	if (msr_peak_min != -99.0)
		document.getElementById("peak_mini").value = msr_peak_min;
	if (msr_peak_max != 8000.0)
		document.getElementById("peak_maxi").value = msr_maxi_min;
}
function msdiag_one()
{
	$(".msmenu").css("display", "none");
	document.getElementById("menu_one").style.display="block";
	if (msr_one_min != -1)
		document.getElementById("peak_mini").value = msr_one_min;
	if (msr_one_max != 16)
		document.getElementById("peak_maxi").value = msr_one_max;
}
function msdiag_two()
{
	$(".msmenu").css("display", "none");
	document.getElementById("menu_two").style.display="block";
	if (msr_two_min != -1)
		document.getElementById("two_mini").value = msr_two_min;
	if (msr_two_max != 16)
		document.getElementById("two_maxi").value = msr_two_max;
}
function msdiag_esp()  {$(".msmenu").css("display", "none");document.getElementById("menu_esp").style.display="block"}
function msdiag_ytd()  {$(".msmenu").css("display", "none");document.getElementById("menu_ytd").style.display="block"}
function msdiag_y19()  {$(".msmenu").css("display", "none");document.getElementById("menu_y19").style.display="block"}
function msdiag_y18()  {$(".msmenu").css("display", "none");document.getElementById("menu_y18").style.display="block"}
function msdiag_5pc()  {$(".msmenu").css("display", "none");document.getElementById("menu_five").style.display="block"}
function msdiag_10pc() {$(".msmenu").css("display", "none");$("#menu_ten").css("display", "block")}

// ancient code
function sb150(){
	if (multi_open_SB150 == 0)
		table_refresh_SB150=1;
}
function sb1k(){
	table_refresh_SB1K=1;
	if (!SB1K_INIT) {
		setInterval(function(){
			if (table_refresh_SB1K) {
	    		SB1K_table.ajax.reload(function(){
				openedRows_SB1K.forEach(row => {
				SB1K_table.row(row).child(sb1K_xls(SB1K_table.row(row).data())).show();
				SB1K_table.row(row).nodes().to$().addClass('shown');
	    		});
				},false);
			}
		}, 300000);
		SB1K_INIT=1;
	}
	SB1K_table.ajax.reload();
}

function lightbox()
{
	var ticker = $(event.target).closest('.view').attr('id').split('_')[0],
	lbx = document.getElementById("stockbox");
	lbx.style.display="block";
		$.ajax({url:"/box/"+ticker, async:"false", complete: function(result){
			$(lbx).append(result.responseText);
		}});
}
function closelbx(){document.getElementById("stockbox").innerHTML=''}

function table_id(id)
{
	if (id == "SB150")
		return SB150_table;
	else if (id == "SB1K")
		return SB1K_table;
}

function full_xls()
{
	var evt = window.event;
	var parent = evt.target.parentNode;
	var ticker,tr,row;
	var found = 0;

	while (parent.tagName != "TABLE") {
		if (parent.tagName == "DIV") {
			var lbx = document.getElementById("LBX");
			var win = window.open('https://www.stockminer.world/XPAGE/' + lbx.className.split("_")[1].split(" ")[0]);
			win.focus();
			return;
		}
		if (parent.tagName == "TR") {
			if (!found) {
				tr = parent;
				found = 1;
			}
		}
		parent = parent.parentNode;
	}

	ticker = $(tr).prev().find("td:eq(1)").text();
	tr = $(tr).prev();
	$.ajax({url:"/search/"+ticker+"/full", async:"false", complete: function(result){
		if (parent.tagName == "DIV") {
			$("#stockbox").append(result.responseText);
		} else {
			row = table_id(parent.id).row(tr);
			row.child(result.responseText).show();
		}
	}});
}

function esp_delta(){}
function esp_month(month){$.ajax({url:"/ESEL/"+month, async:"false", complete: function(result){ESP_table.ajax.reload()}})}

function sb150_xls(d, row, tr){
	$.ajax({url:"/data/XLS/"+d.ticker, async:"false", complete: function(result){
		table_refresh_SB150 = 0;
		row.child(result.responseText).show();
		tr.addClass('shown');
		if (!table_refresh_SB150)
			multi_open_SB150+=1;
		else
			table_refresh_SB150=0;
		openedRows_SB150.add(row.index());
	}});
}

function sb1K_xls(d, row, tr){
	$.ajax({url:"/data/XLS/"+d.ticker, async:"false", complete: function(result){
		table_refresh_SB1K = 0;
		row.child(result.responseText).show();
		tr.addClass('shown');
		if (!table_refresh_SB1K)
			multi_open_SB1K+=1;
		else
			table_refresh_SB1K=0;
		openedRows_SB1K.add(row.index());
	}});
}

function stock_scatter(av)
{
	var series = [], qdiv, c;

	series[0] = {
	    type:'line',
	    name:'Price',
		lineColor:"#07AD73",
		data:JSON.parse(av[2]),
		marker: {enabled:false},
	    states: {hover: {lineWidth:0}},
	    enableMouseTracking:false
	};
	series[1] = {
    	type: 'scatter',
    	name: 'Signals',
    	data: JSON.parse(av[1]),
		color:"rgb(30, 144, 255, 1)",
    	marker: {symbol:"circle",fillColor:"#FF3406",radius:4}
	};
	qdiv = av[3].split("-")[0];
	c = av[3].substr(1);
	$(qdiv).append("<div id="+c+"></div>");
	scatter_chart(c, "Signals", series);
	charts[c].reflow();
}

// ancient, obsolete, may be reusable for other purposes, once i have time to remember what it once did
function signal_chart(ticker, div)
{
	var json = [];
	Highcharts.getJSON('/CSV/'+ticker, function (data) {
	    dataLength = data.length,i = 0;
		for (i; i < dataLength; i += 1) {json.push([data[i][0],data[i][1]])}

	var series = [];
	$.ajax({url:"/SCATTER/"+ticker, async:"false", complete: function(result){
		var actions    = result.responseText.split("\n");
		var a1_blue    = actions[0];
		var a1_yellow  = actions[1];
		var a1_green   = actions[2];
		var a1_red     = actions[3];
		var a4_blue    = actions[4];
		var a4_yellow  = actions[5];
		var a4_green   = actions[6];
		var a4_red     = actions[7];
		var nr_sigsets = 1;

	series[0] = {
	    type:'line',
	    name:'Price',
		lineColor:"#00FFFF",
		data:JSON.parse(av[2]),
		marker: {enabled:false},
	    states: {hover: {lineWidth:0}},
	    enableMouseTracking:false
	};
		if (a1_blue != "[]" && a1_blue != "[" && a1_blue != undefined) {
			a1_blue = JSON.parse(a1_blue);
			series[nr_sigsets++] = {
	        	type: 'scatter',
	        	name: 'Action 1 Close',
	        	data: a1_blue,
				color:"rgb(30, 144, 255, 1)",
	        	marker: {symbol:"circle",fillColor:"dodgerblue",radius:4}
	    	};
		}
		if (a1_green != "[]" && a1_green != "[" && a1_green != undefined) {
			a1_green = JSON.parse(a1_green);
			series[nr_sigsets++] = {
	        	type: 'scatter',
	        	name: 'Action 1 Success',
				color:"green",
	        	data: a1_green,
	        	marker: {symbol:"circle",fillColor:"green",radius:4}
			};
		}

		if (a1_yellow != "[]" && a1_yellow != "[" && a1_yellow != undefined) {
			a1_yellow = JSON.parse(a1_yellow);
			series[nr_sigsets++] = {
	        	type: 'scatter',
	        	name: 'Action 1 Target',
				colorByPoint:true,
				color:"yellow",
	        	data: a1_yellow,
	        	marker: {symbol:"circle",fillColor:"yellow",radius:4}
	    	};
		}

		if (a1_red != "[]" && a1_red != "[" && a1_red != undefined) {
			a1_red = JSON.parse(a1_red);
			series[nr_sigsets++] = {
	        	type: 'scatter',
	        	name: 'Action 1 In Progress',
				color:"red",
	        	data: a1_red,
	        	marker: {symbol:"circle",radius:4}
			};
		}

		if (a4_blue != "[]" && a4_blue != "[" && a4_blue != undefined) {
			a4_blue = JSON.parse(a4_blue);
			series[nr_sigsets++] = {
	        	type: 'scatter',
	        	name: 'Action 4 Close',
				color:"rgb(30, 144, 255, 1)",
	        	data: a4_blue,
	        	marker: {symbol:"circle",fillColor:"dodgerblue",radius:4}
			};
		}

		if (a4_yellow != "[]" && a4_yellow != "[" && a4_yellow != undefined) {
			a4_yellow = JSON.parse(a4_yellow);
			series[nr_sigsets++] = {
	        	type: 'scatter',
	        	name: 'Action 4 Target',
	        	data: a4_yellow,
				color:"yellow",
	        	marker: {symbol:"circle",fillColor:"yellow",radius:4}
			};
		}

		if (a4_green != "[]" && a4_green != "[" && a4_green != undefined) {
			a4_green = JSON.parse(a4_green);
			series[nr_sigsets++] = {
	        	type: 'scatter',
	        	name: 'Action 4 Success',
				color:"green",
	        	data: a4_green,
	        	marker: {symbol:"circle",fillColor:"green",radius:4}
			};
		}

		if (a4_red != "[]" && a4_red != "[" && a4_red != undefined) {
			a4_red = JSON.parse(a4_red);
			series[nr_sigsets++] = {
	        	type: 'scatter',
	        	name: 'Action 4 In Progress',
				color:"red",
	        	data: a4_red,
	        	marker: {symbol:"circle",fillColor:"red",radius:4}
			};
		}
		scatter_chart(div, ticker, series);
	}});
	setInterval(function(){$('.highcharts-point').removeClass("highcharts-point")},200);
});
}

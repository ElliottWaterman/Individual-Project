$('document').ready(function() {
	/*
	 * Format of report table, initial sort time descending (most recent first).
	 */
	var table = new Tabulator("#report-table", {
	    layout:"fitColumns",			//fit columns to width of table
		movableColumns:true,		//allow column order to be changed
		responsiveLayout:"hide",	//hide columns that dont fit on the table
	    columns:[ //set column definitions for imported table data
	        {title:"ID", sorter:"alphanum", bottomCalc:"count", widthGrow:1.5, responsive:9},
	        {title:"Phone Number", sorter:"string", widthGrow:1, responsive:8},
	        {title:"Time Snake Detected", field:"time", widthGrow:1, responsive:0},
	        {title:"Temperature", sorter:"number", align:"center", width:115},
	        {title:"Humidity", sorter:"number", align:"center", width:115},
	        {title:"Weight", sorter:"number", align:"center", width:115},
	        {title:"Snake RFID", field:"rfid", sorter:"alphanum", widthGrow:1, responsive:0},
	        {title:"Skink RFIDs", sorter:"alphanum", widthGrow:3, responsive:7},
	    ],
	    initialSort:[
	        {column:"time", dir:"desc"},	//sort by this first
	    ],
	});
	
	
	
	/* 	
		var table = new Tabulator("#example-table", {
			//data:tabledata,           //load row data from array
			layout:"fitColumns",      //fit columns to width of table
			responsiveLayout:"hide",  //hide columns that dont fit on the table
			tooltips:true,            //show tool tips on cells
			addRowPos:"top",          //when adding a new row, add it to the top of the table
			history:true,             //allow undo and redo actions on the table
			pagination:"local",       //paginate the data
			paginationSize:10,        //allow 10 rows per page of data
			movableColumns:true,      //allow column order to be changed
			resizableRows:true,       //allow row order to be changed
			initialSort:[             //set the initial sort order of the data
				{column:"id", dir:"asc"},
			],
			columns:[                 //define the table columns
				{title:"ID", field:"id", min-width:250},
				{title:"Phone Number", field:"phoneNumber"},
				{title:"Time", field:"time", sorter:"date"},
				{title:"RFID", field:"rfid"},
				{title:"Temperature", field:"temperature", align:"center"},
				{title:"Weight", field:"weight", align:"center"},
			],
			columns:[                 //define the table columns DELETE
				{title:"ID", field:"id", editor:"input"},
				{title:"Phone Number", field:"phoneNumber", editor:true},
				{title:"Time", field:"time", width:95, editor:"select", editorParams:{"Male":"male", "Female":"female"}},
				{title:"RFID", field:"rfid", formatter:"star", align:"center", width:100, editor:true},
				{title:"Temperature", field:"temperature", width:130, editor:"input"},
				{title:"Weight", field:"weight", width:130, sorter:"date", align:"center"},
			],
		});
	
		//var tabledata = xyz;
		table.setData("../SBSBS.csv"); 
	*/
});


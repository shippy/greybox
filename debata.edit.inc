<?php

if ($GLOBALS["cps_debaty"] < 1) {
	pg_achtung($lang["access denied"]);
	return;
}

include_once("i_domeny.inc");

$debata_id = $_REQUEST["debata_id"];

if (!cpdb_fetch_one_row("
	select
		tym_aff.tym_ID as aff_ID,
		tym_aff.nazev as aff_tym,
		dt_aff.body as aff_body,
		tym_neg.tym_ID as neg_ID,
		tym_neg.nazev as neg_tym,
		dt_neg.body as neg_body,
		teze.tx as teze,
		teze.teze_ID as teze_ID,
		debata.datum,
		debata.vitez,
		debata.presvedcive,
		debata.debata_ID,
		soutez.soutez_ID,
		soutez.nazev as soutez,
		soutez.jazyk as jazyk,
		turnaj.turnaj_ID,
		turnaj.nazev as turnaj
	from
		debata
		left join debata_tym dt_aff on dt_aff.debata_ID = debata.debata_ID and dt_aff.pozice = 1
		left join debata_tym dt_neg on dt_neg.debata_ID = debata.debata_ID and dt_neg.pozice = 0
		left join tym tym_aff on dt_aff.tym_ID = tym_aff.tym_ID
		left join tym tym_neg on dt_neg.tym_ID = tym_neg.tym_ID
		left join teze on teze.teze_ID = debata.teze_ID
		left join soutez on soutez.soutez_ID = debata.soutez_ID
		left join turnaj on turnaj.turnaj_ID = debata.turnaj_ID
	where
		debata.debata_ID = :debata_id
	", array(":debata_id"=>$debata_id), $debata)) {
	
	pg_achtung($lang["no record"]);
	return;
}

$GLOBALS["page"]->title = format_date($debata["datum"]) . " " . $debata["aff_tym"] . "/" . $debata["neg_tym"];
pg_head();

// build teams domain
$sdomain_tym = array(
	"select
		tym.tym_ID,
		concat(tym.nazev, ' (', klub.kratky_nazev, ')') as tym
	from
		tym
		left join klub using (klub_ID)
	order by
		tym.nazev
	", array()
);

// selects for debater domains
$select_debaters = "
		(
		select
			clovek.clovek_ID,
			concat_ws(' ', clovek.jmeno, clovek.nick, clovek.prijmeni) as clovek,
			concat_ws(' ', clovek.prijmeni,clovek.jmeno,clovek.nick) as s_clovek
		from
			clovek_tym
			left join clovek using (clovek_ID)
		where
			clovek_tym.aktivni = 1
			and clovek.debater = 1
			%s
		)
		union
		(
		select
			0 as clovek_ID,
			:null as clovek,
			'' as s_clovek
		)
		order by
			s_clovek
	";

// build affirmative debaters domain
if (isset($debata["aff_ID"])) {
	// choose affirmatives from the team
	$sdomain_aff = array(sprintf($select_debaters,"and clovek_tym.tym_ID = :tym_id"), array(":tym_id"=>$debata["aff_ID"], ":null"=>$lang["N/A"]));
} else {
	// choose from all debaters
	$sdomain_aff = array(sprintf($select_debaters,""), array(":null"=>$lang["N/A"]));
}

// build negative debaters domain
if (isset($debata["neg_ID"])) {
	// choose negatives from the team
	$sdomain_neg = array(sprintf($select_debaters,"and clovek_tym.tym_ID = :tym_id"), array(":tym_id"=>$debata["neg_ID"], ":null"=>$lang["N/A"]));
} else {
	// choose from all debaters
	$sdomain_neg = array(sprintf($select_debaters,""), array(":null"=>$lang["N/A"]));
}

// build resolutions domain
$sdomain_teze = array(
	sprintf("select teze_ID, tx as tx from teze %s order by tx", isset($debata["jazyk"]) ? "where jazyk = :jazyk" : ""),
	array(":jazyk"=>$debata["jazyk"])
);

// build competitions domain
$sdomain_souteze = array(
	"select soutez_ID, nazev from soutez where rocnik = :rocnik",
	array(":rocnik"=>$GLOBALS["current_season"])
);

// build tournaments domain
$sdomain_turnaje = array(
	sprintf("
		(select turnaj_ID, nazev, nazev as nazev_s from turnaj %s)
		union
		(select 0, :null, '' as nazev_s)
		order by nazev_s
	", isset($debata["soutez_ID"]) ? "where soutez_ID = :soutez_id" : ""),
	array(":soutez_id"=>$debata["soutez_ID"], ":null"=>$lang["N/A"])
);

cph_p(cphs_link("./?page=debata&debata_id=" . $debata["debata_ID"], $lang["back to debate"]));

// select, ktery k $debata prida pole [an][123]_(ID|kidy)
cpdb_fetch_all("
	select
		clovek_ID, role, kidy
	from
		clovek_debata
	where
		debata_ID = :debata_id
		and find_in_set(role,'a1,a2,a3,n1,n2,n3')
	", array(":debata_id"=>$debata_id), $recnici);
foreach ($recnici as $recnik) {
	$debata += array(
		$recnik["role"] . "_ID" => $recnik["clovek_ID"],
		$recnik["role"] . "_kidy" => $recnik["kidy"]
	);
}

// pridani result = [AN][+-]
$dres = $debata["vitez"] ? "A" : "N";
$dres .= $debata["presvedcive"] ? "+" : "-";
$debata["vysledek"] = $dres;

// pykhen na zakladni vlastnosti
$dpyk = new cpykhen;
$dpyk->form_attr["action"] = "./?page=debata.edit.exec";
$dpyk->form[] = new cpf_db_hidden("debata_id","debata_ID");
$dpyk->form[] = new cpf_text($lang["competition"] . ": ");
$dpyk->form[] = new cpf_db_listbox("soutez_id", $sdomain_souteze, "soutez_ID");
$dpyk->form[] = new cpf_nl;
$dpyk->form[] = new cpf_text($lang["tournament"] . ": ");
$dpyk->form[] = new cpf_db_listbox("turnaj_id", $sdomain_turnaje, "turnaj_ID");
$dpyk->form[] = new cpf_nl;
$dpyk->form[] = new cpf_text($lang["date"] . ": ");
$dpyk->form[] = new cpf_db_textbox("datum","datum",array("size"=>"11"));
$dpyk->form[] = new cpf_text(" " . $lang["yyyy-mm-dd"],array("class"=>"desc"));
$dpyk->form[] = new cpf_nl;
$dpyk->form[] = new cpf_text($lang["place"] . ": ");
$dpyk->form[] = new cpf_db_textbox("misto");
$dpyk->form[] = new cpf_nl;
$dpyk->form[] = new cpf_text($lang["resolution"] . ": ");
$dpyk->form[] = new cpf_db_listbox("teze_id", $sdomain_teze, "teze_ID");

$dpyk->form[] = new cpf_text($lang["affirmative"],array("class"=>"p_h2"));
$dpyk->form[] = new cpf_text($lang["team"] . ": ");
$dpyk->form[] = new cpf_db_listbox("aff_id", $sdomain_tym, "aff_ID");
$dpyk->form[] = new cpf_nl;
$dpyk->form[] = new cpf_text($lang["1st speaker"] . ": ");
$dpyk->form[] = new cpf_db_listbox("a1_id", $sdomain_aff, "a1_ID");
$dpyk->form[] = new cpf_text(" " . $lang["kidy"] . ": ");
$dpyk->form[] = new cpf_db_textbox("a1_kidy","a1_kidy",array("size"=>"3"));
$dpyk->form[] = new cpf_nl;
$dpyk->form[] = new cpf_text($lang["2nd speaker"] . ": ");
$dpyk->form[] = new cpf_db_listbox("a2_id", $sdomain_aff, "a2_ID");
$dpyk->form[] = new cpf_text(" " . $lang["kidy"] . ": ");
$dpyk->form[] = new cpf_db_textbox("a2_kidy","a2_kidy",array("size"=>"3"));
$dpyk->form[] = new cpf_nl;
$dpyk->form[] = new cpf_text($lang["3rd speaker"] . ": ");
$dpyk->form[] = new cpf_db_listbox("a3_id", $sdomain_aff, "a3_ID");
$dpyk->form[] = new cpf_text(" " . $lang["kidy"] . ": ");
$dpyk->form[] = new cpf_db_textbox("a3_kidy","a3_kidy",array("size"=>"3"));
$dpyk->form[] = new cpf_nl;

$dpyk->form[] = new cpf_text($lang["negative"],array("class"=>"p_h2"));
$dpyk->form[] = new cpf_text($lang["team"] . ": ");
$dpyk->form[] = new cpf_db_listbox("neg_id", $sdomain_tym, "neg_ID");
$dpyk->form[] = new cpf_nl;
$dpyk->form[] = new cpf_text($lang["1st speaker"] . ": ");
$dpyk->form[] = new cpf_db_listbox("n1_id", $sdomain_neg, "n1_ID");
$dpyk->form[] = new cpf_text(" " . $lang["kidy"] . ": ");
$dpyk->form[] = new cpf_db_textbox("n1_kidy","n1_kidy",array("size"=>"3"));
$dpyk->form[] = new cpf_nl;
$dpyk->form[] = new cpf_text($lang["2nd speaker"] . ": ");
$dpyk->form[] = new cpf_db_listbox("n2_id", $sdomain_neg, "n2_ID");
$dpyk->form[] = new cpf_text(" " . $lang["kidy"] . ": ");
$dpyk->form[] = new cpf_db_textbox("n2_kidy","n2_kidy",array("size"=>"3"));
$dpyk->form[] = new cpf_nl;
$dpyk->form[] = new cpf_text($lang["3rd speaker"] . ": ");
$dpyk->form[] = new cpf_db_listbox("n3_id", $sdomain_neg, "n3_ID");
$dpyk->form[] = new cpf_text(" " . $lang["kidy"] . ": ");
$dpyk->form[] = new cpf_db_textbox("n3_kidy","n3_kidy",array("size"=>"3"));
$dpyk->form[] = new cpf_nl;

$dpyk->form[] = new cpf_text($lang["result"],array("class"=>"p_h2"));
$dpyk->form[] = new cpf_text($lang["result"] . ": ");
$dpyk->form[] = new cpf_db_listbox_array("vysledek", array(
	"A+" => $lang["aff"] . " " . $lang["3:0"],
	"A-" => $lang["aff"] . " " . $lang["2:1"],
	"N+" => $lang["neg"] . " " . $lang["3:0"],
	"N-" => $lang["neg"] . " " . $lang["2:1"]
));
$dpyk->form[] = new cpf_text(" " . $lang["applies only if there are no adjudicators filled in"], array("class"=>"desc"));
$dpyk->form[] = new cpf_nl;

$dpyk->form[] = new cpf_button("save",$lang["save"]);
$dpyk->form[] = new cpf_button("delete",$lang["delete"]);
$dpyk->load_this($debata);
$dpyk->render();

cph_h2($lang["adjudicators"] . "/" . $lang["organizers"]);

// inherited organizers
if (cpdb_fetch_all("select clovek.clovek_ID, clovek.jmeno, clovek.nick, clovek.prijmeni from clovek_turnaj left join clovek using(clovek_ID) where clovek_turnaj.turnaj_ID = :turnaj_id and clovek_turnaj.role = 'o'", array(":turnaj_id"=>$debata["turnaj_ID"]), $orgs)) {
	de_html($orgs);
	cph_table_head(array($lang["role"],$lang["person"]),array("class"=>"visible"));
	foreach ($orgs as $clovek) {
		cph_table_row(array(
			$lang["inherited organizer"],
			cphs_link("./?page=clovek&clovek_id=" . $clovek["clovek_ID"], join_name($clovek["jmeno"],$clovek["nick"],$clovek["prijmeni"]))
		));
	}
	cph_table_end();
}

// adjudicators/organizers
$klub_clovek = new cpykhen;
$klub_clovek->form_attr["action"] = "./?page=debata.clovek.edit.exec";
$klub_clovek->select = "select dc_ID, clovek_ID, role, rozhodnuti, presvedcive from clovek_debata where debata_ID = :debata_id and find_in_set(role,'r,o') order by role";
$klub_clovek->select_args = array(":debata_id"=>$debata_id);
$klub_clovek->form[] = new cpf_hidden("debata_id",$debata_id);
$klub_clovek->form[] = new cpf_db_hidden("dc_id","dc_ID");
$klub_clovek->form["role"] = new cpf_db_listbox_array("role", array("r"=>$lang["adjudicator"], "o"=>$lang["organizer"]));
$klub_clovek->form["select"] = new cpf_db_listbox("clovek_id",
	array("select clovek.clovek_ID, concat_ws(' ', jmeno, nick, prijmeni) from clovek order by prijmeni, jmeno, nick", array()), "clovek_ID");
$klub_clovek->form["link"] = new cpf_db_link("clovek_ID","./?page=clovek&clovek_id=%s",$lang["link"]);
$klub_clovek->form["b1"] = new cpf_button("save",$lang["save"]);
$klub_clovek->form["b2"] = new cpf_button("delete",$lang["delete"]);
$klub_clovek->render_select();

$klub_clovek->form["select"] = new cpf_db_listbox("clovek_id",
	array("select clovek.clovek_ID, concat_ws(' ', jmeno, nick, prijmeni) from rozhodci left join clovek using(clovek_ID) where rozhodci.rocnik = :rocnik and find_in_set(:jazyk, rozhodci.jazyk) order by prijmeni, jmeno, nick", array(":rocnik"=>$GLOBALS["current_season"], ":jazyk"=>$debata["jazyk"])), "clovek_ID");
$klub_clovek->form["role"] = new cpf_db_listbox_array("role", array("r"=>$lang["adjudicator"]));
$klub_clovek->form["b1"] = new cpf_button("add",$lang["add"]);
unset($klub_clovek->form["b2"]);
unset($klub_clovek->form["link"]);
$klub_clovek->init_force();
$klub_clovek->load_new();
$klub_clovek->render_hidden($lang["new adjudicator"]);

$klub_clovek->form["select"] = new cpf_db_listbox("clovek_id",
	array("select clovek.clovek_ID, concat_ws(' ', jmeno, nick, prijmeni) from clovek order by prijmeni, jmeno, nick", array()), "clovek_ID");
$klub_clovek->form["role"] = new cpf_db_listbox_array("role", array("o"=>$lang["organizer"]));
$klub_clovek->init_force();
$klub_clovek->load_new();
$klub_clovek->render_hidden($lang["new organizer"]);

?>
<?php
pg_head();
$teze_id = $_REQUEST["teze_id"];

if ($GLOBALS['cps_teze'] >= 1) {
	$teze = new cpykhen;
	$teze->select = "select * from teze where teze_ID = :teze_id";
	$teze->select_args = array (":teze_id" => $teze_id);
	$teze->form_attr["action"] = "./?page=teze.edit.exec";
	$teze->form[] = new cpf_db_hidden("teze_id","teze_ID");
	$teze->form[] = new cpf_text($lang["language"] . ": ");
	$teze->form[] = new cpf_db_listbox_array("jazyk",array("cz"=>"cz", "en"=>"en", "de"=>"de", "fr"=>"fr"));
	$teze->form[] = new cpf_nl;
	$teze->form[] = new cpf_text($lang["resolution"] . ": ");
	$teze->form[] = new cpf_db_textbox("tx","tx",array("size"=>60));
	$teze->form[] = new cpf_nl;
	$teze->form[] = new cpf_text($lang["comment"] . ": ");
	$teze->form[] = new cpf_db_textarea("komentar","komentar",array("rows"=>4, "cols"=>60));
	$teze->form[] = new cpf_nl;
	$teze->form[] = new cpf_button("save",$lang["save"]);
	$teze->form[] = new cpf_button("delete",$lang["delete"]);
	$teze->render_select();
} else {
	cpdb_fetch_one_row("select * from teze where teze_ID = :teze_id", array (":teze_id" => $teze_id), $teze);
	cph_p($lang["language"] . ": " . $teze["jazyk"]);
	cph_p($lang["resolution"] . ": " . $teze["tx"]);
	if (isset($teze["komentar"])) cph_p($lang["comment"] . ":" . cphs_br() . $teze["komentar"]);
}

if (cpdb_fetch_all("select soutez.soutez_ID as soutez, nazev from soutez_teze left join soutez using (soutez_ID) where teze_ID = :teze_id", array(":teze_id" => $teze_id), $souteze) > 0) {
	cph_tag("h2",array(),$lang["official in"]);
	cph_table_head(array($lang["competition"],""),array("class"=>"visible"));
	foreach ($souteze as $soutez) {
		cph_table_row(array($soutez["nazev"], cphs_link("./?page=soutez.detaily&soutez_id=" .  $soutez["soutez"],$lang["details"])));
	}
	cph_table_end();
}

// debates w/ teams, resolutions, result, link
cph_h2($lang["debated"]);

cpdb_fetch_all("
	select
		tym_aff.tym_ID as aff_ID,
		tym_aff.nazev as aff_tym,
		tym_neg.tym_ID as neg_ID,
		tym_neg.nazev as neg_tym,
		datum,
		vitez,
		presvedcive,
		debata.debata_ID
	from
		debata
		left join debata_tym dt_aff on dt_aff.debata_ID = debata.debata_ID
		left join debata_tym dt_neg on dt_neg.debata_ID = debata.debata_ID
		left join tym tym_aff on dt_aff.tym_ID = tym_aff.tym_ID
		left join tym tym_neg on dt_neg.tym_ID = tym_neg.tym_ID
	where
		dt_aff.pozice = 1
		and dt_neg.pozice = 0
		and debata.teze_ID = :teze_id
	order by
		datum desc
	", array(":teze_id"=>$teze_id),$r_debaty);
cph_table_head(array($lang["date"],$lang["aff"],$lang["neg"],$lang["result"],""),array("class"=>"visible"));
foreach ($r_debaty as $debata) {
	if ($debata["vitez"] == 1) {
		$result = $lang["aff"];
	} else {
		$result = $lang["neg"];
	}
	$result .= " ";
	if ($debata["presvedcive"] == 1) {
		$result .= $lang["persuasively"];
	} else {
		$result .= $lang["less persuasively"];
	}
	cph_table_row(array(
		format_date($debata["datum"]),
		cphs_link("./?page=tym&tym_id=" . $debata["aff_ID"],$debata["aff_tym"]),
		cphs_link("./?page=tym&tym_id=" . $debata["neg_ID"],$debata["neg_tym"]),
		$result,
		cphs_link("./?page=debata&debata_id=" . $debata["debata_ID"], $lang["details"])
	));
}
cph_table_end();

?>
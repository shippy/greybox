<?php

$klub_id = $_REQUEST["klub_id"];

pg_head();

if ($GLOBALS["cps_lidi"] < 1) {
	pg_achtung($lang['access denied']);
	return;
}
pg_head();

$clovek_f = new cpykhen;
$clovek_f->formattr["action"] = "./?page=clovek.add";
$clovek_f->form[] = new cpf_text($lang["name"] . ": ");
$clovek_f->form[] = new cpf_db_textbox("jmeno");
$clovek_f->form[] = new cpf_nl;
$clovek_f->form[] = new cpf_text($lang["surname"] . ": ");
$clovek_f->form[] = new cpf_db_textbox("prijmeni");
$clovek_f->form[] = new cpf_nl;
$clovek_f->form[] = new cpf_text($lang["nick"] . ": ");
$clovek_f->form[] = new cpf_db_textbox("nick");
$clovek_f->form[] = new cpf_nl;
$clovek_f->form[] = new cpf_text($lang["club"] . ": ");
$clovek_f->form[] = new cpf_db_listbox("klub_id",array(
	"select klub_ID, nazev from klub union select 0, :no_club",array(":no_club"=>$lang["no club"])),"klub_ID");
$clovek_f->form[] = new cpf_nl;
$clovek_f->form[] = new cpf_text($lang["born"] . ": ");
$clovek_f->form[] = new cpf_db_textbox("narozen");
$clovek_f->form[] = new cpf_nl;
$clovek_f->form[] = new cpf_text($lang["comment"] . ": ");
$clovek_f->form[] = new cpf_db_textarea("komentar","komentar",array("rows"=>4, "cols"=>60));
$clovek_f->form[] = new cpf_nl;
$clovek_f->form[] = new cpf_text($lang["active debater"] . ": ");
$clovek_f->form[] = new cpf_db_listbox_array("aktivni",array(1 => $lang['yes'], 0 => $lang['no']));
$clovek_f->form[] = new cpf_nl;
$clovek_f->form[] = new cpf_button("add",$lang["add"]);
$clovek_f->load_this(array("klub_ID"=>$klub_id));
$clovek_f->render();


?>
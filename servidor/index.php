<?php
if ($_SERVER['REQUEST_METHOD'] == 'POST'){
	// cuando recibe una petición POST  crea un archivo con el formato file_ + dirección IP del cliente + hora de la petición post en el server
	// y almacena toda la información que le pasa el cliente en el post al cliente
	file_put_contents('file_'.$_SERVER['REMOTE_ADDR'].'_' .date('m-d-Y_hia').'.txt', file_get_contents('php://input'));
	

}
if ($_SERVER['REQUEST_METHOD'] == 'GET'){
	//cuando se hace una petición GET se listan todos los archivos del directorio (menos el index.php)
	if ($handle = opendir('.')) {
    		echo "<ul>";
    		while (false !== ($entry = readdir($handle))) {
        			if ($entry != "." && $entry != ".." && $entry != "index.php") {
            			echo '<li><a href="'.$entry.'">'.$entry.'</a></li>';
        			}
    		}
    		echo "</ul>";
    		closedir($handle);
	}
}
?>

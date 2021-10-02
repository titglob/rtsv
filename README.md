Le project rtsv permet d'insérer dans votre projet certaines fonctions afin de faciler le debogge d'applications temps réel. Les résulats d'analyse peuvent provenir de multiples sources et sont collectées par un serveur. Des fichiers waves analysables avec gtkwave ansi que des messages sequence chart générés avec msc_latex package sont générés (les msc sont écrits dans une ou plusieurs pages d'un fichier pdf). L'api est utilisable directement dans du code C mais on peut aussi utiliser des scripts en ligne de commande (si le but est juste de faire un doc).

Définition du besoin:
---------------------
Traquer les évenements internes et/ou externes sous forme de message sequence chart et/ou de waves, entre des processus non synchronizés (ex: de différents devices). La vision vcd est l'unique outil permettant de visualiser simplement des changements de valeur de nombreuses variables, mais n'affiche pas la causalité des évenements contrairement aux msc, qui demeurent un outil de visualisation plus 'human frendly', mais dans laquelle la notion de temps (qu'il est possible de représenter), importe moins que la causalité des évenements.

L'ideal est d'utiliser des abstractions d'objects ou de placer dans les interfaces bas niveau ces triggers pour éviter d'envahir au maximum le code utilisateur.

L'idée générale est de fournir une api *unique*, sous forme textuelle ou d'api C, permettant de mettre en valeur des changements d'état principaux dans le code et de remonter les informations au serveur rtsv afin de les transcrire en msc, vcd, mais aussi pourquoi pas en dot, xhtml, ... 

Les méchanismes utilisés pour acheminer les informations de logs entre processus utilisent à priori des chemins indépendants.

On a besoin de plusieurs briques logicielles:
   - une librarie indépendante du serveur, portable sur des archi sans os embarquées qui servirait à :
      * convertir structure <-> buffer <-> command text en fonction de la commande.
      * gérer la notion de temps synchronisé (rt_time_t). Cette notion doit exprimer correctement la causalité et aussi donner être représentative du
        temps qui s'écoule, en choissant une base de temps commune (implique de connaitre les fréquences des processeurs).
      => int rt_msg_to_buf(buf, cmd, id1, id2, text)

   - le rtsv server, qui doit pouvoir:
      * lire sur un ou plusieurs fichiers texte ou binaire
      * générer une ou plusieurs sorties ou être pluggé sur d'autres outil (ex: msc_tracer de pragmadev).
      * faire le rapprochement des send_msg avec les recv_msg
      * trier par ordre croissant les messages queués
      * maintenir le current level de la simulation
      * permettre de grouper les objects en groupes ou sous groupes
      * support d'un mode de batch pour pouvoir avancer en pas a pas
      * utiliser des fichiers de configuration pour du formattage spécifique ex:
         * affiner l'ordre des process
         * choisir un formattage différents pour les objets (couleur, style)

   - une interface C pour instrumenter le code, avec une implémentation spécifique librarie spécifique
   - une implémentation spéficique pour chaque processeur pour
      * implémenter les fonctions d'api, ex:

          rt_inittask((t_object_id)tid, "OBJNAME"):
               int n = rt_pack_msg(rt_buffer, RTSV_DEF_CMD_INITTASK, id, 0, obj_name)
               mbx->send(rt_buffer, n);

      * créer et gére le canal de communication vers le serveur

Pour les msc, on voudra faire apparaître graphiquement les informations suivantes:
  - la notion de groupe, qui servira a préfixer les noms des objects et à les trier par ordre alphabétique (ou via un fichier de conf) dans le diagramme
  - les processus, préfixés du nom du groupe
  - envoie et reception d'un message : on veut faire apparaitre les croisements de messages, les dates.
  - l'ajout d'actions
  - les changements d'états
  - l'ajout de commentaires
  - les timers (start, stop, timeout).
  - timelife : desactivé, activé, suspendu.
  - les objects de synchronization, style sémaphore, mutex, ...
  - detecter les pertes de messages ou encore l'apparition de messages.
  - optionnellement, on pourra fonctionner en mode untimed, cad que l'on s'interresse uniquement
    à l'ordre partiel des évenements, et non a la durée.
  - on a besoin d'un algorithme de coupure de page, scalable pour accepter des tailles de pages configurables.
  - les msc pouvant être volumineux, il est necessaire de pouvoir définir les plages sur lesquelles l'utilisateur souhaite observer les évenements.
    => directement corréler à l'algo de cesure de pages.

Pour les waves, on veut: TDB

Choix d'implémentation:
-----------------------

L'unité de temps est le 'Systime', configurable. On suppose connu le temps de transmission d'un message d'un processeur à l'autre: transmitTime.
La notion de temps choisié doit garantir l'ordre partiel grace au MAX dans rt_sync(), mais tiens compte aussi du temps qui sécoule grace a un vco par exemple.
L'avantage de cette grandeur est que l'on représente bien la causalité (a précé b => time(a) < time(b)), les écarts de temps, et que l'on peut choisir plus finement le *retard de processing*, cad la durée à partir de laquelle les messages non récus sont considérés comme perdus. La granularité pour le temps doit être configurable.

    rt_time_t rt_time():
       return vco() x RATIO + offTime

    rt_time_t rt_sync()(extTime):
       locTime = vco() x RATIO.
       ajustedTime = MAX(extTime + transmitTime, locTime + offTime) // afin de garantir la causalité.
       offTime = ajustedTime - locTime // reajust the systemTime 
       return ajustedTime

Dans rtsv, au fur et a mesure, les messages qui arrivent au host sont lus, des objects sont créés, puis insérés dans une queue par ordre du plus ancien au plus vieux.
Si le plus vieux a une date "inférieure" à celle du plus récent d'au moins une durée configurable, alors le message est déqueué et l'on process pour ce message toutes les sorties actives.

Les objects générés peuvent être classés en groupes, nécessaires pour organiser et afficher plus clairement les données ou taches. On n'interdit pas les objets provenant de fichiers différents de faire partie d'un même groupe. l'id de chaque object reste unique. Au sein d'un groupe, les objects sont triés par ordre d'arrivé. Les groupes sont eux même des objets, donc possèdent un id unique. Graphiquement parlant, l'affichage des instances de msc ou des symbols vcd seront regroupés. Pour les msc comme pour les vcd, déclarer à postérieurie une instance pose problème car il faut connaitre la hierachie des le début. Ainsi, pour les vcd, le fichier de définition sera réellement écrit à la fin (sauf mode fifo), et pour les msc, à chaque nouvelle page l'ordre par groupe est rétablit mais sur une même page.




sudo apt install texlive-pstricks
sudo apt install texlive-latex-extra
